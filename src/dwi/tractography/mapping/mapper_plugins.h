/* Copyright (c) 2008-2023 the MRtrix3 contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Covered Software is provided under this License on an "as is"
 * basis, without warranty of any kind, either expressed, implied, or
 * statutory, including, without limitation, warranties that the
 * Covered Software is free of defects, merchantable, fit for a
 * particular purpose or non-infringing.
 * See the Mozilla Public License v. 2.0 for more details.
 *
 * For more details, see http://www.mrtrix.org/.
 */

#ifndef __dwi_tractography_mapping_mapper_plugins_h__
#define __dwi_tractography_mapping_mapper_plugins_h__


#include "image.h"
#include "types.h"
#include "fixel/dataset.h"
#include "interp/linear.h"
#include "math/sphere/SH.h"
#include "math/sphere/set/assigner.h"

#include "dwi/tractography/streamline.h"
#include "dwi/tractography/mapping/twi_stats.h"


namespace MR {
  namespace DWI {
    namespace Tractography {
      namespace Mapping {





        class DixelMappingPlugin
        {
          public:
            DixelMappingPlugin (const Math::Sphere::Set::Assigner& dir2dixel) :
              dir2dixel (dir2dixel) { }
            Math::Sphere::Set::index_type operator() (const Eigen::Vector3d& d) const { return dir2dixel (d); }
          private:
            const Math::Sphere::Set::Assigner& dir2dixel;
        };



        class TODMappingPlugin
        {
          public:
            TODMappingPlugin (const size_t N) :
                generator (new Math::Sphere::SH::aPSF<float> (Math::Sphere::SH::LforN (N))),
                sh (Eigen::Matrix<default_type, Eigen::Dynamic, 1>::Zero (N)) { }
            TODMappingPlugin (const TODMappingPlugin& that) :
                generator (that.generator),
                sh (Eigen::Matrix<default_type, Eigen::Dynamic, 1>::Zero (that.sh.size())) { }
            template <class UnitVectorType>
            void operator() (const UnitVectorType& d) { (*generator) (sh, d); }
            const Eigen::Matrix<default_type, Eigen::Dynamic, 1>& operator()() const { return sh; }
          private:
            std::shared_ptr<Math::Sphere::SH::aPSF<float>> generator;
            Eigen::Matrix<default_type, Eigen::Dynamic, 1> sh;
        };



        // Currently the if statement for whether or not mask information is available
        //   will be performed for every single voxel intersection of every streamline
        // TODO See if there's a way to speed this up...
        // TODO The prospect of having modified mask information means there's
        //   actually 3 options here...
        // TODO See if this can be made private
        class FixelMappingPlugin : public MR::Fixel::IndexImage
        {
          public:
          using fixel_index_type = MR::Fixel::index_type;
          using dixel_index_type = Math::Sphere::Set::index_type;
            FixelMappingPlugin (const MR::Fixel::Dataset& dataset, const bool exhaustive = true) :
                MR::Fixel::IndexImage (dataset),
                dataset (dataset),
                exhaustive (exhaustive) { }
            FixelMappingPlugin (const FixelMappingPlugin& that) :
                MR::Fixel::IndexImage (that),
                dataset (that.dataset),
                exhaustive (that.exhaustive) { }
            fixel_index_type operator() (const Eigen::Vector3i& voxel, const Eigen::Vector3d& dir);
            fixel_index_type nfixels() const { return dataset.nfixels(); }
          private:
            const MR::Fixel::Dataset& dataset;
            const bool exhaustive;
        };






        class TWIImagePluginBase
        {
          public:
            TWIImagePluginBase (const std::string& input_image, const tck_stat_t track_statistic) :
                statistic (track_statistic),
                interp (Image<float>::open (input_image).with_direct_io()),
                backtrack (false) { }

            TWIImagePluginBase (Image<float>& input_image, const tck_stat_t track_statistic) :
                statistic (track_statistic),
                interp (input_image),
                backtrack (false) { }

            TWIImagePluginBase (const TWIImagePluginBase& that) :
                statistic (that.statistic),
                interp (that.interp),
                backtrack (that.backtrack),
                backtrack_mask (that.backtrack_mask) { }

            virtual ~TWIImagePluginBase() { }

            virtual TWIImagePluginBase* clone() const = 0;

            void set_backtrack();

            virtual void load_factors (const Streamline<>&, vector<default_type>&) const = 0;


          protected:
            const tck_stat_t statistic;

            //Image<float> voxel;
            // Each instance of the class has its own interpolator for obtaining values
            //   in a thread-safe fashion
            mutable Interp::Linear<Image<float>> interp;

            // For handling backtracking for endpoint-based track-wise statistics
            bool backtrack;
            mutable Image<bool> backtrack_mask;

            // Helper functions; find the last point on the streamline from which valid image information can be read
            ssize_t                        get_end_index (const Streamline<>&, const bool) const;
            const Streamline<>::point_type get_end_point (const Streamline<>&, const bool) const;

        };




        class TWIScalarImagePlugin : public TWIImagePluginBase
        {
          public:
            TWIScalarImagePlugin (const std::string& input_image, const tck_stat_t track_statistic) :
                TWIImagePluginBase (input_image, track_statistic)
            {
              assert (statistic != ENDS_CORR);
              if (!((interp.ndim() == 3) || (interp.ndim() == 4 && interp.size(3) == 1)))
                throw Exception ("Scalar image used for TWI must be a 3D image");
              if (interp.ndim() == 4)
                interp.index(3) = 0;
            }

            TWIScalarImagePlugin (const TWIScalarImagePlugin& that) :
                TWIImagePluginBase (that)
            {
              if (interp.ndim() == 4)
                interp.index(3) = 0;
            }

            TWIScalarImagePlugin* clone() const override
            {
              return new TWIScalarImagePlugin (*this);
            }

            void load_factors (const Streamline<>&, vector<default_type>&) const override;
        };




        class TWIFODImagePlugin : public TWIImagePluginBase
        {
          public:
            TWIFODImagePlugin (const std::string& input_image, const tck_stat_t track_statistic) :
                TWIImagePluginBase (input_image, track_statistic),
                sh_coeffs (interp.size(3)),
                precomputer (new Math::Sphere::SH::PrecomputedAL<default_type> ())
            {
              if (track_statistic == ENDS_CORR)
                throw Exception ("Cannot use ends_corr track statistic with an FOD image");
              Math::Sphere::SH::check (Header (interp));
              precomputer->init (Math::Sphere::SH::LforN (sh_coeffs.size()));
            }

            TWIFODImagePlugin (const TWIFODImagePlugin& that) = default;

            TWIFODImagePlugin* clone() const override
            {
              return new TWIFODImagePlugin (*this);
            }

            void load_factors (const Streamline<>&, vector<default_type>&) const override;

          private:
            mutable Eigen::Matrix<default_type, Eigen::Dynamic, 1> sh_coeffs;
            std::shared_ptr<Math::Sphere::SH::PrecomputedAL<default_type>> precomputer;
        };




        class TWDFCStaticImagePlugin : public TWIImagePluginBase
        {
          public:
            TWDFCStaticImagePlugin (Image<float>& input_image) :
                TWIImagePluginBase (input_image, ENDS_CORR) { }

            TWDFCStaticImagePlugin (const TWDFCStaticImagePlugin& that) = default;

            TWDFCStaticImagePlugin* clone() const override
            {
              return new TWDFCStaticImagePlugin (*this);
            }

            void load_factors (const Streamline<>&, vector<default_type>&) const override;
        };




        class TWDFCDynamicImagePlugin : public TWIImagePluginBase
        {
          public:
            TWDFCDynamicImagePlugin (Image<float>& input_image, const vector<float>& kernel, const ssize_t timepoint) :
                TWIImagePluginBase (input_image, ENDS_CORR),
                kernel (kernel),
                kernel_centre ((kernel.size()-1) / 2),
                sample_centre (timepoint) { }

            TWDFCDynamicImagePlugin (const TWDFCDynamicImagePlugin& that) = default;

            TWDFCDynamicImagePlugin* clone() const override
            {
              return new TWDFCDynamicImagePlugin (*this);
            }

            void load_factors (const Streamline<>&, vector<default_type>&) const override;

          protected:
            const vector<float> kernel;
            const ssize_t kernel_centre, sample_centre;
        };




      }
    }
  }
}

#endif



