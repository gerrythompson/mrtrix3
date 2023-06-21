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

#include "math/bessel.h"

namespace MR {
namespace Math {
namespace Bessel {

const double coef_aI0[12] = {-.07660547252839144951,
                             1.92733795399380827000,
                             .22826445869203013390,
                             .01304891466707290428,
                             .00043442709008164874,
                             .00000942265768600193,
                             .00000014340062895106,
                             .00000000161384906966,
                             .00000000001396650044,
                             .00000000000009579451,
                             .00000000000000053339,
                             .00000000000000000245};

const double coef_bI0[21] = {.07575994494023796, .00759138081082334,  .00041531313389237,
                             .00001070076463439, -.00000790117997921, -.00000078261435014,
                             .00000027838499429, .00000000825247260,  -.00000001204463945,
                             .00000000155964859, .00000000022925563,  -.00000000011916228,
                             .00000000001757854, .00000000000112822,  -.00000000000114684,
                             .00000000000027155, -.00000000000002415, -.00000000000000608,
                             .00000000000000314, -.00000000000000071, .00000000000000007};

const double coef_cI0[22] = {
    .05449041101410882,  .00336911647825569,  .00006889758346918,  .00000289137052082,
    .00000020489185893,  .00000002266668991,  .00000000339623203,  .00000000049406022,
    .00000000001188914,  -.00000000003149915, -.00000000001321580, -.00000000000179419,
    .00000000000071801,  .00000000000038529,  .00000000000001539,  -.00000000000004151,
    -.00000000000000954, .00000000000000382,  .00000000000000176,  -.00000000000000034,
    -.00000000000000027, .00000000000000003};

const double coef_aI1[11] = {-0.001971713261099859,
                             0.407348876675464810,
                             0.034838994299959456,
                             0.001545394556300123,
                             0.000041888521098377,
                             0.000000764902676483,
                             0.000000010042493924,
                             0.000000000099322077,
                             0.000000000000766380,
                             0.000000000000004741,
                             0.000000000000000024};

const double coef_bI1[21] = {-0.02846744181881479, -0.01922953231443221, -0.00061151858579437,
                             -0.00002069971253350, 0.00000858561914581,  0.00000104949824671,
                             -0.00000029183389184, -0.00000001559378146, 0.00000001318012367,
                             -0.00000000144842341, -0.00000000029085122, 0.00000000012663889,
                             -0.00000000001664947, -0.00000000000166665, 0.00000000000124260,
                             -0.00000000000027315, 0.00000000000002023,  0.00000000000000730,
                             -0.00000000000000333, 0.00000000000000071,  -0.00000000000000006};

const double coef_cI1[22] = {
    0.02857623501828014,  -0.00976109749136147, -0.00011058893876263, -0.00000388256480887,
    -0.00000025122362377, -0.00000002631468847, -0.00000000383538039, -0.00000000055897433,
    -0.00000000001897495, 0.00000000003252602,  0.00000000001412580,  0.00000000000203564,
    -0.00000000000071985, -0.00000000000040836, -0.00000000000002101, 0.00000000000004273,
    0.00000000000001041,  -0.00000000000000382, -0.00000000000000186, 0.00000000000000033,
    0.00000000000000028,  -0.00000000000000003};

} // namespace Bessel
} // namespace Math
} // namespace MR
