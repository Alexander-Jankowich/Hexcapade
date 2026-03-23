//
//  GLInputController.cpp
//  Geometry Lab
//
//  This class buffers in input from the devices and converts it into its
//  semantic meaning. If your game had an option that allows the player to
//  remap the control keys, you would store this information in this class.
//  That way, the main game scene does not have to keep track of the current
//  key mapping.
//
//  Author: Walker M. White
//  Version: 1/29/24
//
#include <cugl/cugl.h>
#include "GLInputController.h"

using namespace cugl;

#pragma mark Input Constants

/** The key to use for reseting the game */
#define RESET_KEY KeyCode::R
/** The key for toggling the debug display */
#define DEBUG_KEY KeyCode::D
/** The key for exitting the game */
#define EXIT_KEY  KeyCode::ESCAPE

/** How close we need to be for a multi touch */
#define NEAR_TOUCH      100
/** The key for the event handlers */
#define LISTENER_KEY      1

/** How fast a double click must be in milliseconds */
#define DOUBLE_CLICK    400

std::vector<std::vector<Vec2>> X_GESTURE = {
    {
        {1026.695312, 544.195312},
        {1026.695312, 543.953125},
        {1012.515625, 529.476562},
        {969.273438, 487.671875},
        {902.906250, 426.726562},
        {847.929688, 377.593750},
        {815.179688, 349.468750},
        {794.531250, 333.039062},
        {779.953125, 321.648438},
        {774.023438, 317.710938},
        {772.375000, 317.421875},
        {772.328125, 319.281250},
        {772.328125, 319.281250}
    },
    {
        {786.523438, 544.015625},
        {788.179688, 543.070312},
        {813.078125, 526.609375},
        {864.593750, 501.273438},
        {943.875000, 457.046875},
        {988.789062, 430.570312},
        {1036.046875, 401.820312},
        {1097.742188, 360.757812},
        {1111.539062, 351.742188},
        {1116.757812, 348.515625},
        {1117.578125, 348.007812},
        {1117.578125, 348.007812}
    }
};

std::vector<std::vector<Vec2>> HEAL_GESTURE = {
    {
        {799.082031, 316.132843},
        {799.679688, 316.734406},
        {807.484375, 323.906281},
        {826.242188, 341.160187},
        {852.984375, 365.937531},
        {894.757812, 408.417969},
        {919.265625, 435.593781},
        {939.195312, 460.546906},
        {950.011719, 474.015686},
        {955.183594, 480.449249},
        {960.617188, 486.992218},
        {964.894531, 492.007874},
        {966.800781, 494.242249},
        {967.921875, 495.601593},
        {968.675781, 496.359436},
        {969.109375, 496.761780},
        {969.109375, 496.761780}
    },
    {
        {795.699219, 403.976593},
        {796.460938, 405.593781},
        {801.906250, 412.023468},
        {808.433594, 419.136749},
        {835.414062, 447.140686},
        {848.859375, 459.750000},
        {860.589844, 470.492249},
        {872.089844, 480.949249},
        {882.832031, 490.027405},
        {892.128906, 497.687531},
        {899.187500, 503.429749},
        {904.375000, 507.878937},
        {910.617188, 513.406311},
        {912.457031, 515.308655},
        {913.503906, 516.074280},
        {913.761719, 516.335938},
        {913.761719, 516.335938}
    },
    {
        {784.847656, 499.066437},
        {784.847656, 498.152405},
        {784.847656, 494.964905},
        {784.847656, 491.468811},
        {784.847656, 485.953186},
        {784.847656, 478.257874},
        {784.847656, 466.500061},
        {784.847656, 452.058655},
        {784.847656, 447.300842},
        {784.847656, 428.250031},
        {784.847656, 413.656281},
        {784.847656, 397.332062},
        {784.847656, 378.417999},
        {785.070312, 361.250031},
        {785.917969, 344.394562},
        {787.148438, 328.058624},
        {788.351562, 312.664093},
        {789.531250, 298.175812},
        {790.679688, 284.394562},
        {791.925781, 264.355499},
        {792.179688, 259.273468},
        {792.761719, 246.390640},
        {792.761719, 237.949234},
        {792.761719, 232.621124},
        {792.761719, 226.812531},
        {792.761719, 220.750031},
        {792.761719, 215.339859},
        {792.761719, 211.187515},
        {792.761719, 205.660172},
        {792.761719, 199.609390},
        {792.761719, 193.523453},
        {792.761719, 187.125015},
        {792.761719, 181.449219},
        {792.761719, 176.203140},
        {792.761719, 170.984406},
        {792.761719, 166.433609},
        {793.097656, 160.644547},
        {793.097656, 158.019547},
        {793.097656, 157.082047},
        {793.410156, 154.222656},
        {793.410156, 152.675797},
        {793.691406, 151.292999},
        {793.691406, 150.128922},
        {793.691406, 149.406265},
        {793.691406, 148.375015},
        {793.691406, 147.199234},
        {793.691406, 146.132812},
        {793.691406, 145.390625},
        {793.691406, 144.863281},
        {793.691406, 144.335938},
        {793.328125, 144.417984}
    }
};


std::vector<cugl::Vec2> V_GESTURE = {
    {947.566650, 554.066650},
    {955.754150, 544.787476},
    {957.391602, 525.683350},
    {961.758362, 496.754181},
    {966.670837, 465.641663},
    {971.037537, 436.712494},
    {976.495789, 411.058380},
    {982.500000, 391.408386},
    {988.504150, 382.129181},
    {994.508362, 381.583344},
    {1000.512512, 386.495880},
    {1009.245850, 408.875000},
    {1027.258301, 450.358368},
    {1074.745728, 549.154175},
    {1095.487549, 578.083374},
    {1106.404175, 592.820862},
    {1112.408203, 599.370850},
    {1117.320801, 601.554199},
    {1121.687500, 603.191650},
    {1123.324951, 603.191650},
    {1123.324951, 603.191650}
};

std::vector<cugl::Vec2> V_GESTURE_1 = {
    {947.566650, 554.066650},
    {955.754150, 544.787476},
    {957.391602, 525.683350},
    {961.758362, 496.754181},
    {966.670837, 465.641663},
    {971.037537, 436.712494},
    {976.495789, 411.058380},
    {982.500000, 391.408386},
    {988.504150, 382.129181},
    {994.508362, 381.583344},
    {1000.512512, 386.495880},
    {1009.245850, 408.875000},
    {1027.258301, 450.358368},
    {1074.745728, 549.154175},
    {1095.487549, 578.083374},
    {1106.404175, 592.820862},
    {1112.408203, 599.370850},
    {1117.320801, 601.554199},
    {1121.687500, 603.191650},
    {1123.324951, 603.191650},
    {1123.324951, 603.191650}
};

std::vector<cugl::Vec2> V_GESTURE_2 = {
    {1055.641602, 552.429199},
    {1054.004150, 544.787476},
    {1053.458374, 528.412476},
    {1039.266724, 477.649994},
    {1032.170898, 449.812500},
    {1026.712524, 427.433319},
    {1022.345825, 408.875000},
    {1017.979126, 393.045837},
    {1011.429199, 381.583344},
    {1005.424988, 375.033386},
    {999.420837, 372.304169},
    {995.600037, 371.758362},
    {988.504150, 377.762543},
    {984.137451, 391.408386},
    {979.225037, 415.970825},
    {973.766663, 453.087494},
    {968.854126, 492.387482},
    {961.758362, 522.954163},
    {954.116699, 539.329224},
    {948.112488, 548.062500},
    {945.383362, 552.429199},
    {944.837524, 554.066650},
    {944.837524, 554.612488},
    {944.837524, 554.612488}
};

std::vector<cugl::Vec2> V_GESTURE_3 = {
    {957.391602, 536.599976},
    {962.850037, 524.591675},
    {963.941650, 507.125000},
    {970.491699, 479.833374},
    {976.495789, 453.087494},
    {983.591614, 430.708374},
    {990.141663, 412.695862},
    {996.691650, 401.233337},
    {1002.695740, 397.412506},
    {1009.245850, 397.412506},
    {1016.341675, 405.054169},
    {1028.349976, 427.433319},
    {1053.458374, 484.200012},
    {1064.375000, 507.125000},
    {1074.199951, 524.045837},
    {1082.933350, 533.325012},
    {1087.845825, 537.691650},
    {1091.120850, 538.237488},
    {1091.666626, 538.237488},
    {1091.666626, 538.237488}
};

std::vector<cugl::Vec2> V_GESTURE_4 = {
    {1052.366577, 543.695862},
    {1054.550049, 527.866699},
    {1054.550049, 512.583374},
    {1054.550049, 489.658356},
    {1049.637451, 439.987488},
    {1046.362549, 415.970825},
    {1043.633301, 394.683319},
    {1041.449951, 379.399994},
    {1040.904053, 371.758362},
    {1040.904053, 370.666656},
    {1040.904053, 371.758362},
    {1040.904053, 375.033386},
    {1038.175049, 381.037506},
    {1033.808350, 389.770874},
    {1027.258301, 404.508362},
    {1020.162537, 425.250000},
    {1009.791626, 448.174988},
    {998.875061, 468.370850},
    {987.958252, 482.562500},
    {978.679199, 492.387482},
    {971.037537, 497.845856},
    {965.033264, 499.483337},
    {965.033264, 500.029175},
    {965.033264, 500.029175}
};

std::vector<cugl::Vec2> V_GESTURE_5 = {
    {1000.512512, 534.962524},
    {1003.787537, 513.675049},
    {1005.424988, 489.658356},
    {1010.883362, 461.274994},
    {1019.070862, 433.983368},
    {1028.349976, 412.695862},
    {1037.629150, 396.320831},
    {1046.362549, 384.312500},
    {1054.004150, 376.670868},
    {1061.099976, 374.487488},
    {1070.379150, 376.125000},
    {1081.841553, 396.866669},
    {1093.304077, 425.250000},
    {1101.491577, 450.904175},
    {1108.041626, 474.375000},
    {1114.591675, 493.479187},
    {1122.233398, 508.762512},
    {1128.237549, 519.133362},
    {1133.150024, 524.591675},
    {1135.879150, 525.683350},
    {1137.516724, 526.229187},
    {1137.516724, 524.591675},
    {1137.516724, 524.591675}
};

std::vector<cugl::Vec2> CIRCLE = {
    {1015.250000, 435.075012},
    {1019.070862, 429.616669},
    {1024.529175, 420.883331},
    {1026.166626, 411.058380},
    {1032.716553, 390.316711},
    {1035.991699, 363.570862},
    {1037.629150, 336.279175},
    {1038.175049, 310.625000},
    {1037.629150, 284.970825},
    {1032.170898, 261.500000},
    {1021.799927, 245.670868},
    {980.316589, 223.837555},
    {944.291687, 214.558319},
    {906.629150, 209.645874},
    {878.245850, 209.645874},
    {863.508362, 214.012512},
    {854.775085, 226.020844},
    {848.225037, 243.487488},
    {843.312561, 270.233337},
    {842.766663, 308.441681},
    {849.862488, 354.837494},
    {871.150024, 395.229156},
    {900.079102, 424.158386},
    {941.016663, 445.991669},
    {986.320862, 462.912506},
    {1038.720825, 474.375000},
    {1077.474976, 477.104187},
    {1082.933350, 477.104187},
    {1082.933350, 477.104187},
};

std::vector<cugl::Vec2> CIRCLE_1 = {
    {1029.441650, 448.720856},
    {1025.074951, 449.266693},
    {1017.979126, 450.904175},
    {1012.520813, 450.904175},
    {1003.787537, 450.904175},
    {991.233276, 450.904175},
    {929.554199, 425.795868},
    {905.537537, 403.416656},
    {893.529175, 383.766663},
    {890.799988, 362.479218},
    {890.254150, 336.825012},
    {892.983276, 310.625000},
    {907.174988, 285.516663},
    {925.187500, 262.045868},
    {945.929077, 242.941681},
    {967.216675, 227.112488},
    {988.504150, 218.379181},
    {1008.700012, 216.195831},
    {1028.895874, 218.924988},
    {1051.820801, 240.758331},
    {1076.383301, 272.416656},
    {1098.216675, 307.895874},
    {1112.954102, 336.825012},
    {1114.045898, 363.570862},
    {1104.766602, 373.395874},
    {1089.483398, 387.587494},
    {1060.008301, 413.241669},
    {1020.708313, 438.895874},
    {985.775024, 458.000031},
    {968.308289, 467.279175},
    {965.579163, 468.370850},
    {967.216675, 468.370850},
    {972.129089, 468.370850},
    {972.129089, 468.370850},
};

std::vector<cugl::Vec2> CIRCLE_2 = {
    {1040.358276, 432.345886},
    {1045.816650, 417.608368},
    {1045.816650, 414.879181},
    {1049.091675, 401.779175},
    {1054.004150, 381.037506},
    {1055.641602, 358.112488},
    {1056.187378, 335.733368},
    {1056.187378, 312.808319},
    {1051.820801, 289.883362},
    {1040.358276, 267.504181},
    {1024.529175, 248.399994},
    {1004.333313, 234.754211},
    {978.679199, 223.837555},
    {947.020813, 213.466675},
    {915.908264, 206.370850},
    {885.887512, 203.641663},
    {844.950012, 217.833374},
    {832.941650, 238.029205},
    {824.754089, 265.320862},
    {820.387512, 293.704224},
    {819.295837, 318.812500},
    {819.295837, 335.733368},
    {821.479187, 343.375000},
    {835.125000, 357.020844},
    {862.962524, 382.129181},
    {902.262512, 409.966675},
    {944.291687, 425.795868},
    {977.041687, 430.708374},
    {999.420837, 430.162506},
    {1014.704163, 428.524994},
    {1026.166626, 424.704163},
    {1034.354126, 423.066681},
    {1041.449951, 420.883331},
    {1041.449951, 420.883331},
};

std::vector<cugl::Vec2> CIRCLE_3 = {
    {1025.074951, 457.454193},
    {1013.612488, 457.454193},
    {1011.429199, 457.454193},
    {1005.424988, 457.454193},
    {967.762512, 444.354156},
    {939.379150, 426.341675},
    {918.637512, 406.145844},
    {904.991699, 383.766663},
    {899.533325, 359.750000},
    {898.987488, 333.549988},
    {901.170837, 303.529175},
    {917.000000, 277.329224},
    {937.195862, 257.133331},
    {961.212402, 241.304169},
    {985.775024, 229.295837},
    {1010.883362, 224.929169},
    {1034.900024, 226.020844},
    {1060.008301, 240.758331},
    {1085.116699, 267.504181},
    {1107.495850, 297.524994},
    {1120.050049, 322.087555},
    {1122.779175, 341.191681},
    {1122.779175, 354.837494},
    {1103.675049, 386.495880},
    {1073.654175, 415.970825},
    {1035.445801, 442.716675},
    {1004.333313, 460.183350},
    {986.866577, 468.370850},
    {980.862488, 472.191650},
    {979.770752, 473.283356},
    {979.770752, 474.375000},
    {979.225037, 474.920837},
    {979.225037, 474.920837},
};

std::vector<cugl::Vec2> CIRCLE_4 = {
    {1102.583374, 442.170868},
    {1105.858398, 434.529175},
    {1109.133301, 421.975006},
    {1109.133301, 420.337494},
    {1110.224976, 414.333344},
    {1111.316650, 404.508362},
    {1111.862549, 388.133362},
    {1111.862549, 367.937500},
    {1111.862549, 348.833374},
    {1111.316650, 333.549988},
    {1110.224976, 320.449951},
    {1108.587646, 310.625000},
    {1105.858398, 303.529175},
    {1102.037476, 298.616669},
    {1097.125000, 295.341675},
    {1088.937500, 290.975006},
    {1078.566650, 285.516663},
    {1064.375000, 279.512543},
    {1043.633301, 270.233337},
    {1016.341675, 258.770844},
    {971.583313, 242.941681},
    {959.029175, 241.850006},
    {949.749939, 241.850006},
    {938.833313, 244.579163},
    {923.549988, 254.950012},
    {907.174988, 266.958344},
    {894.075012, 278.420868},
    {886.433350, 288.245880},
    {883.158325, 296.979156},
    {882.612488, 307.350037},
    {882.612488, 323.179169},
    {884.250000, 345.012512},
    {892.983276, 367.391663},
    {904.991699, 387.587494},
    {925.733337, 407.237488},
    {950.295837, 425.250000},
    {975.404175, 437.258331},
    {1004.333313, 444.899994},
    {1033.262451, 447.629181},
    {1064.375000, 445.991669},
    {1069.287476, 440.533356},
    {1069.287476, 440.533356},
};

std::vector<cugl::Vec2> CIRCLE_5 = {
    {912.633362, 419.245880},
    {897.895813, 418.700012},
    {892.983276, 416.516663},
    {884.250000, 409.966675},
    {872.241699, 399.049988},
    {859.687500, 385.950012},
    {849.316650, 372.304169},
    {841.129150, 356.475006},
    {834.579163, 338.462494},
    {830.212524, 321.541718},
    {829.666687, 307.350037},
    {829.666687, 295.887512},
    {832.395752, 284.424988},
    {840.037415, 271.870819},
    {848.770813, 257.679169},
    {859.141663, 242.395844},
    {882.066650, 212.375000},
    {894.075012, 198.729187},
    {906.629150, 187.266663},
    {919.729065, 179.625000},
    {934.466614, 175.258362},
    {952.479187, 174.166687},
    {972.129089, 174.166687},
    {996.691650, 184.537476},
    {1023.983337, 210.191711},
    {1052.912476, 244.033325},
    {1074.745728, 266.412506},
    {1085.116699, 274.600006},
    {1090.029053, 279.512543},
    {1094.941650, 286.062500},
    {1097.670776, 295.887512},
    {1099.308350, 311.170837},
    {1099.854126, 331.912506},
    {1070.379150, 373.395874},
    {1035.991699, 394.683319},
    {994.508362, 413.241669},
    {957.391602, 426.341675},
    {932.829163, 431.799988},
    {920.275024, 431.799988},
    {911.541565, 430.708374},
    {898.441650, 423.612488},
    {878.245850, 410.512512},
    {860.779175, 399.049988},
    {854.229065, 394.683319},
    {854.775085, 395.229156},
    {860.779175, 395.229156},
    {860.779175, 395.229156},
};

std::vector<cugl::Vec2> CIRCLE_6 = {
    {912.633362, 419.245880},
    {897.895813, 418.700012},
    {892.983276, 416.516663},
    {884.250000, 409.966675},
    {872.241699, 399.049988},
    {859.687500, 385.950012},
    {849.316650, 372.304169},
    {841.129150, 356.475006},
    {834.579163, 338.462494},
    {830.212524, 321.541718},
    {829.666687, 307.350037},
    {829.666687, 295.887512},
    {832.395752, 284.424988},
    {840.037415, 271.870819},
    {848.770813, 257.679169},
    {859.141663, 242.395844},
    {882.066650, 212.375000},
    {894.075012, 198.729187},
    {906.629150, 187.266663},
    {919.729065, 179.625000},
    {934.466614, 175.258362},
    {952.479187, 174.166687},
    {972.129089, 174.166687},
    {996.691650, 184.537476},
    {1023.983337, 210.191711},
    {1052.912476, 244.033325},
    {1074.745728, 266.412506},
    {1085.116699, 274.600006},
    {1090.029053, 279.512543},
    {1094.941650, 286.062500},
    {1097.670776, 295.887512},
    {1099.308350, 311.170837},
    {1099.854126, 331.912506},
    {1070.379150, 373.395874},
    {1035.991699, 394.683319},
    {994.508362, 413.241669},
    {957.391602, 426.341675},
    {932.829163, 431.799988},
    {920.275024, 431.799988},
    {911.541565, 430.708374},
    {898.441650, 423.612488},
    {878.245850, 410.512512},
    {860.779175, 399.049988},
    {854.229065, 394.683319},
    {854.775085, 395.229156},
    {860.779175, 395.229156},
    {860.779175, 395.229156},
};

std::vector<cugl::Vec2> CIRCLE_7 = {
    {1125.508301, 437.804169},
    {1112.954102, 435.075012},
    {1109.133301, 435.620819},
    {1107.495850, 435.620819},
    {1102.583374, 435.620819},
    {1094.941650, 435.620819},
    {1085.662476, 437.258331},
    {1074.199951, 439.987488},
    {1058.916626, 441.625000},
    {1007.608337, 442.716675},
    {978.679199, 439.441681},
    {955.208313, 430.162506},
    {938.833313, 419.245880},
    {923.549988, 404.508362},
    {904.445740, 382.129181},
    {886.979187, 355.383331},
    {874.424988, 330.274994},
    {866.783325, 308.441681},
    {863.508362, 288.791656},
    {863.508362, 270.779175},
    {863.508362, 254.950012},
    {868.420837, 241.304169},
    {876.062500, 229.295837},
    {884.250000, 218.379181},
    {894.620850, 208.008362},
    {906.083313, 197.637512},
    {920.820862, 189.995850},
    {939.379150, 184.537476},
    {990.687500, 183.991638},
    {1021.799927, 196.545837},
    {1052.912476, 215.104156},
    {1081.841553, 233.116669},
    {1108.587646, 248.399994},
    {1132.604126, 263.137512},
    {1151.708374, 276.783386},
    {1163.716675, 289.883362},
    {1169.720825, 303.529175},
    {1171.904175, 319.358368},
    {1171.358398, 335.733368},
    {1166.991699, 350.470825},
    {1150.070801, 370.120819},
    {1117.320801, 396.866669},
    {1074.745728, 422.520844},
    {1037.083252, 436.712494},
    {1014.704163, 441.079193},
    {1004.333313, 441.079193},
    {983.045837, 441.079193},
    {962.850037, 438.895874},
    {935.558350, 426.341675},
    {914.816589, 414.333344},
    {910.450012, 411.604156},
    {910.450012, 411.604156},
};

std::vector<cugl::Vec2> CIRCLE_8 = {
    {1164.262451, 421.975006},
    {1154.437500, 424.158386},
    {1150.616699, 425.795868},
    {1137.516724, 431.254181},
    {1116.229126, 437.258331},
    {1092.212524, 440.533356},
    {1059.462646, 439.987488},
    {1019.616699, 424.704163},
    {979.770752, 393.591675},
    {955.208313, 358.658386},
    {947.566650, 330.820862},
    {947.566650, 305.712494},
    {953.570740, 280.058380},
    {971.037537, 252.766663},
    {996.691650, 230.387512},
    {1026.166626, 215.649994},
    {1056.733398, 210.191711},
    {1084.570801, 210.737488},
    {1135.879150, 245.125000},
    {1157.712524, 272.416656},
    {1171.904175, 302.983368},
    {1179.545898, 335.733368},
    {1180.091675, 365.754181},
    {1172.995850, 391.954224},
    {1149.525024, 418.154175},
    {1122.779175, 438.350006},
    {1099.308350, 449.812500},
    {1088.391724, 451.995880},
    {1086.754150, 451.995880},
    {1086.754150, 452.541656},
    {1086.754150, 452.541656},
};


std::vector<cugl::Vec2> Z_GESTURE = {
    {1073.000000, 320.000000},
    {1077.000000, 320.000000},
    {1092.000000, 319.000000},
    {1116.000000, 321.000000},
    {1137.000000, 323.000000},
    {1156.000000, 324.000000},
    {1167.000000, 324.000000},
    {1171.000000, 323.000000},
    {1172.000000, 322.000000},
    {1173.000000, 322.000000},
    {1175.000000, 320.000000},
    {1177.000000, 319.000000},
    {1178.000000, 314.000000},
    {1174.000000, 306.000000},
    {1168.000000, 295.000000},
    {1154.000000, 280.000000},
    {1136.000000, 262.000000},
    {1118.000000, 244.000000},
    {1106.000000, 229.000000},
    {1096.000000, 215.000000},
    {1088.000000, 207.000000},
    {1085.000000, 204.000000},
    {1085.000000, 203.000000},
    {1096.000000, 203.000000},
    {1122.000000, 204.000000},
    {1159.000000, 208.000000},
    {1203.000000, 212.000000},
    {1233.000000, 215.000000},
    {1235.000000, 215.000000},
    {1247.000000, 217.000000},
    {1248.000000, 217.000000},
    {1249.000000, 217.000000},
    {1249.000000, 217.000000},
};

std::vector<cugl::Vec2> Z_GESTURE_1 = {
    {884.960938, 329.718750},
    {884.718750, 329.718750},
    {889.515625, 329.718750},
    {923.039062, 329.718750},
    {978.851562, 329.718750},
    {1003.710938, 329.718750},
    {1065.617188, 329.718750},
    {1099.968750, 327.781250},
    {1126.390625, 320.921875},
    {1133.742188, 316.390625},
    {1138.570312, 310.703125},
    {1137.515625, 302.273438},
    {1116.906250, 282.007812},
    {1058.359375, 244.335938},
    {972.656250, 197.726562},
    {896.804688, 159.812500},
    {855.085938, 140.195312},
    {829.867188, 127.296875},
    {804.429688, 112.117188},
    {802.398438, 109.171875},
    {802.398438, 107.695312},
    {806.437500, 107.367188},
    {821.187500, 107.875000},
    {862.679688, 112.476562},
    {942.953125, 117.953125},
    {1019.195312, 123.007812},
    {1093.835938, 128.257812},
    {1139.953125, 131.398438},
    {1168.171875, 132.828125},
    {1186.945312, 133.992188},
    {1193.585938, 134.617188},
    {1195.375000, 135.375000},
    {1195.664062, 136.750000},
    {1194.632812, 138.625000},
    {1194.632812, 138.625000}
};

std::vector<cugl::Vec2> Z_GESTURE_2 = {
    {955.195312, 425.328125},
    {959.554688, 425.328125},
    {1003.992188, 425.328125},
    {1052.109375, 424.976562},
    {1109.164062, 418.890625},
    {1149.437500, 410.109375},
    {1171.859375, 403.585938},
    {1186.625000, 398.039062},
    {1192.210938, 394.687500},
    {1193.078125, 391.898438},
    {1184.898438, 384.515625},
    {1159.343750, 368.070312},
    {1115.617188, 341.218750},
    {1030.257812, 289.718750},
    {1014.578125, 280.250000},
    {990.859375, 265.671875},
    {972.867188, 254.867188},
    {964.101562, 248.945312},
    {961.210938, 246.585938},
    {961.117188, 245.492188},
    {961.632812, 244.453125},
    {965.398438, 244.187500},
    {980.484375, 244.187500},
    {1017.750000, 243.515625},
    {1083.320312, 242.843750},
    {1155.625000, 242.843750},
    {1210.359375, 242.257812},
    {1245.937500, 241.671875},
    {1267.140625, 241.671875},
    {1283.156250, 241.671875},
    {1284.640625, 241.671875},
    {1285.156250, 241.671875},
    {1285.679688, 241.671875},
    {1286.734375, 241.671875},
    {1288.054688, 241.671875},
    {1289.187500, 241.937500},
    {1289.187500, 241.937500}
};

std::vector<cugl::Vec2> Z_GESTURE_3 = {
    {906.179688, 470.203125},
    {906.640625, 470.203125},
    {912.953125, 470.203125},
    {936.445312, 470.203125},
    {977.179688, 470.203125},
    {1014.828125, 470.203125},
    {1038.421875, 469.804688},
    {1053.906250, 468.515625},
    {1062.609375, 466.617188},
    {1065.156250, 465.078125},
    {1065.453125, 462.125000},
    {1060.875000, 450.046875},
    {1046.164062, 420.132812},
    {1023.531250, 380.414062},
    {1010.750000, 359.562500},
    {990.687500, 332.523438},
    {964.703125, 302.687500},
    {940.968750, 282.179688},
    {920.945312, 269.773438},
    {912.507812, 266.195312},
    {901.945312, 261.351562},
    {894.687500, 258.468750},
    {891.718750, 257.289062},
    {891.226562, 256.929688},
    {894.437500, 256.929688},
    {911.492188, 258.835938},
    {953.445312, 264.562500},
    {1008.289062, 272.781250},
    {1069.921875, 282.234375},
    {1116.976562, 288.179688},
    {1138.609375, 290.921875},
    {1149.671875, 292.562500},
    {1183.148438, 296.101562},
    {1194.414062, 296.898438},
    {1199.882812, 296.898438},
    {1201.523438, 296.898438},
    {1202.812500, 296.898438},
    {1203.453125, 297.320312},
    {1203.367188, 298.273438},
    {1202.078125, 299.898438},
    {1202.078125, 299.898438}
};


// The screen is divided into four zones: Left and Right
//
//   |---------------|
//   |       |       |
//   | L     |     R |
//   |       |       |
//   |---------------|
//
// The meaning of any touch depends on the zone it begins in.

/** The portion of the screen used for the left zone */
#define LEFT_ZONE       0.5f
/** The portion of the screen used for the right zone */
#define RIGHT_ZONE      0.5f

#pragma mark Input Control
/**
 * Creates a new input controller.
 *
 * This constructor DOES NOT attach any listeners, as we are not
 * ready to do so until the scene is created. You should call
 * the {@link #init} method to initialize the scene.
 */
InputController::InputController() :
_active(false),
_currDown(false),
_prevDown(false),
_mouseDown(false),
_touchDown(false),
_touchID(-1),
_mouseKey(0),
_joystick(false),
_pickupPressed(false),
_resetPressed(false),
_lastGesture(SpellType::NONE),
_isAiming(false),
_releaseSpell(false),
_timeSinceCast(0)
{
}


/**
 * Initializes the control to support mouse or touch.
 *
 * This method attaches all of the listeners. It tests which
 * platform we are on (mobile or desktop) to pick the right
 * listeners.
 *
 * This method will fail (return false) if the listeners cannot
 * be registered or if there is a second attempt to initialize
 * this controller
 *
 * @return true if the initialization was successful
 */
bool InputController::init(std::shared_ptr<JsonValue> values, const Rect bounds,
                            std::function<cugl::Vec2(const cugl::Vec2&)> screenToWorld) {
    _screenToWorld = screenToWorld;
    recognizer = cugl::PGestureRecognizer::alloc();
    recognizer -> addGesture("V",  X_GESTURE );
    recognizer -> addGesture("Z", HEAL_GESTURE);
//    recognizer->addGesture("V", { V_GESTURE_1 });
//    recognizer->addGesture("V", { V_GESTURE_2 });
//    recognizer->addGesture("V", { V_GESTURE_3 });
//    recognizer->addGesture("V", { V_GESTURE_4 });
//    recognizer->addGesture("V", { V_GESTURE_5 });
//    recognizer->addGesture("C", { CIRCLE });
//    recognizer->addGesture("C", { CIRCLE_1 });
//    recognizer->addGesture("C", { CIRCLE_2 });
//    recognizer->addGesture("C", { CIRCLE_3 });
//    recognizer->addGesture("C", { CIRCLE_4 });
//    recognizer->addGesture("C", { CIRCLE_5 });
//    recognizer->addGesture("C", { CIRCLE_6 });
//    recognizer->addGesture("C", { CIRCLE_7 });
//    recognizer->addGesture("C", { CIRCLE_8 });
//    recognizer->addGesture("Z", { Z_GESTURE });
//    recognizer->addGesture("Z", { Z_GESTURE_1 });
//    recognizer->addGesture("Z", { Z_GESTURE_2 });
//    recognizer->addGesture("Z", { Z_GESTURE_3 });

    recognizer->set_accuracy(0.84f);

    // Update the gesturemap with more spells later on
    _gesturemap = {
        {"V", SpellType::FIREBALL},
        {"C", SpellType::FREEZE},
        { "Z", SpellType::HEAL }
    };

    _joyradius = values->get("joystick")->getInt("radius");
    _deadzone = values->get("joystick")->getInt("dead_zone");

    
    //
#ifndef CU_TOUCH_SCREEN
    _sbounds = bounds;
    //
    Size display = Application::get()->getDisplaySize();
    _tbounds = Rect(0,0,display.width,display.height);
        
    createZones();
    clearTouchInstance(_ltouch);
    clearTouchInstance(_rtouch);
    clearTouchInstance(_mtouch);
#else
    Size display = Application::get()->getDrawableSize();
    _tbounds = Rect(0,0,display.width,display.height);
    _sbounds = _tbounds;
    createZones();
    clearTouchInstance(_ltouch);
    clearTouchInstance(_rtouch);
    clearTouchInstance(_mtouch);
    
#endif


    return true;
}



/**
 * Disposes this input controller, deactivating all listeners.
 *
 * As the listeners are deactived, the user will not be able to
 * monitor input until the controller is reinitialized with the
 * {@link #init} method.
 */
void InputController::dispose() {
#ifndef CU_TOUCH_SCREEN
    Input::deactivate<Keyboard>();
    Mouse* mouse = Input::get<Mouse>();
    mouse->removePressListener(LISTENER_KEY);
    mouse->removeReleaseListener(LISTENER_KEY);
    mouse->removeDragListener(LISTENER_KEY);
#else
    Touchscreen* touch = Input::get<Touchscreen>();
    touch->removeBeginListener(LISTENER_KEY);
    touch->removeEndListener(LISTENER_KEY);
#endif
    _active = false;
}


/**
 * Updates the input controller for the latest frame.
 *
 * It might seem weird to have this method given that everything
 * is processed with call back functions.  But we need some way
 * to synchronize the input with the animation frame.  Otherwise,
 * how can we know what was the touch location *last frame*?
 * Maybe there has been no callback function executed since the
 * last frame. This method guarantees that everything is properly
 * synchronized.
 * 
 * @param dt the time (in seconds) since this was last called
 */
void InputController::update(float dt) {
    readInput();

    // Update _isAiming
    _timeSinceCast += dt;
    if (_timeSinceCast > 10000 && _isAiming) {
        _isAiming = false;
        _releaseSpell = true;
        currentStroke.clear();
        CULog("Angle: %.2f %.2f", _spellAngle.x, _spellAngle.y);
    }
}

/**
 * Reads the input for this player and converts the result into game logic.
 *
 * This is an example of polling input.  Instead of registering a listener,
 * we ask the controller about its current state.  When the game is running,
 * it is typically best to poll input instead of using listeners.  Listeners
 * are more appropriate for menus and buttons (like the loading screen).
 */
void InputController::readInput(){
#ifndef CU_TOUCH_SCREEN
    Keyboard* keys = Input::get<Keyboard>();
    if (keys) {
        // Keyboard input for movement
        bool up = keys->keyDown(KeyCode::ARROW_UP) || keys->keyDown(KeyCode::W);
        bool down = keys->keyDown(KeyCode::ARROW_DOWN) || keys->keyDown(KeyCode::S);
        bool left = keys->keyDown(KeyCode::ARROW_LEFT) || keys->keyDown(KeyCode::A);
        bool right = keys->keyDown(KeyCode::ARROW_RIGHT) || keys->keyDown(KeyCode::D);
        static bool upPressed = false;
        static bool downPressed = false;
        static bool leftPressed = false;
        static bool rightPressed = false;

        if (up && !upPressed) {
            _move.y = 1;
            upPressed = true;
        }
        if (down && !downPressed) {
            _move.y = -1;
            downPressed = true;
        }
        if (!up && !down) {
            _move.y = 0;
        }
        upPressed = up;
        downPressed = down;

        if (left && !leftPressed) {
            _move.x = -1;
            leftPressed = true;
        }
        if (right && !rightPressed) {
            _move.x = 1;
            rightPressed = true;
        }
        if (!left && !right) {
            _move.x = 0;
        }
        leftPressed = left;
        rightPressed = right;

        _move.normalize();


        // Keyboard input for item pickup
        bool pickup = keys->keyDown(KeyCode::E);
        static bool pickupDown = false;

        _pickupPressed = pickup && !pickupDown;
        pickupDown = pickup;


        // Keyboard inputs for spells
        bool space = keys->keyDown(KeyCode::SPACE);
        bool shift = keys->keyDown(KeyCode::LEFT_SHIFT);
        static bool spacePressed = false;
        static bool shiftPressed = false;
        if (space && !spacePressed) {
            _lastGesture = SpellType::FIREBALL;
            _spellAngle.setZero();
        }
        spacePressed = space;

        if (shift && !shiftPressed) {
            _lastGesture = SpellType::FREEZE;
            _spellAngle.setZero();
        }
        shiftPressed = shift;


        // Keyboard input for reset
        bool r = keys->keyDown(KeyCode::R);
        static bool rPressed = false;

        _resetPressed = r && !rPressed;
        rPressed = r;
    }

#endif // !CU_TOUCH_SCREEN
};

#pragma mark Touch Callbacks / Controls

/**
 * Defines the zone boundaries, so we can quickly categorize touches.
 */
void InputController::createZones() {
    _lzone = _sbounds;
    _lzone.size.width *= LEFT_ZONE;
    _rzone = _sbounds;
    _rzone.size.width *= RIGHT_ZONE;
    _rzone.origin.x = _sbounds.origin.x+_sbounds.size.width-_rzone.size.width;
}

/**
 * Populates the initial values of the input TouchInstance
 */
void InputController::clearTouchInstance(TouchInstance& touchInstance) {
    touchInstance.touchids.clear();
    touchInstance.position = Vec2::ZERO;
}

/**
 * Returns the correct zone for the given position.
 *
 * See the comments above for a description of how zones work.
 *
 * @param  pos  a position in screen coordinates
 *
 * @return the correct zone for the given position.
 */
InputController::Zone InputController::getZone(const Vec2 pos) const {
    if (_lzone.contains(pos)) {
        return Zone::LEFT;
    } else if (_rzone.contains(pos)) {
        return Zone::RIGHT;
    } else if (_tbounds.contains(pos)) {
        return Zone::MAIN;
    }
    return Zone::UNDEFINED;
}

/**
 * Returns the scene location of a touch
 *
 * Touch coordinates are inverted, with y origin in the top-left
 * corner. This method corrects for this and scales the screen
 * coordinates down on to the scene graph size.
 *
 * @return the scene location of a touch
 */
Vec2 InputController::touch2Screen(const Vec2 pos) const {
    return _screenToWorld(pos);
}

Vec2 InputController::mouse2Screen(const Vec2 pos) const {
    return _screenToWorld(pos);
}

/**
 * Processes movement for the floating joystick.
 *
 * This will register movement as left or right (or neither).  It
 * will also move the joystick anchor if the touch position moves
 * too far.
 *
 * @param  pos  the current joystick position
 */
void InputController::processJoystick(const cugl::Vec2 pos) {
    _joystickpos = pos;
    Vec2 diff = pos - _joycenter;
    float dist = diff.length();

    // Clamp to joystick radius
    if (dist > _joyradius) {
        Vec2 excess = diff;
        diff.normalize();
        diff *= _joyradius;

        excess -= diff;
        // Move joystick in excess direction
        _joycenter += excess;
    }

    if (dist > _deadzone) {
        _joystick = true;

        // Scale to [-1,1] range without normalizing
        float scale = 1.0f / _joyradius;

        float x = diff.x * scale;
        float y = diff.y * scale;

        // Clamp just in case
        x = clampf(x, -1.0f, 1.0f);
        y = clampf(y, -1.0f, 1.0f);

        _move = Vec2(x, y);
    } else {
        _joystick = false;
        _move = Vec2(0,0);
    }
}

void InputController::processGestureEnd(const cugl::Vec2 pos) {
    if (_isAiming) {
        _isAiming = false;
        _releaseSpell = true;
        _spellAngle = pos - _touchStart;
        CULog("Angle: %.2f %.2f", _spellAngle.x, _spellAngle.y);
        return;
    }

    _spellAngle.setZero();
    currentStroke.push_back(pos);
    if (currentStroke.size() > 3) {
        try {
            if((currentStroke.size() > 3) & !secondStroke.empty() & !thirdStroke.empty()){
                std::string gesture = recognizer->match({ currentStroke });
                std::string gesture2 = recognizer -> match({ currentStroke, secondStroke });
                std::string gesture3 = recognizer -> match({ currentStroke, secondStroke, thirdStroke });
                if(recognizer -> similarity(gesture,{ currentStroke }) > recognizer -> similarity(gesture2, {currentStroke, secondStroke})){
                    if(recognizer -> similarity(gesture,{ currentStroke }) > recognizer -> similarity(gesture3, {currentStroke, secondStroke, thirdStroke})){
                        _lastGesture = _gesturemap[gesture];
                    }
                    _lastGesture = _gesturemap[gesture3];
                }else{
                    if(recognizer -> similarity(gesture2,{ currentStroke, secondStroke }) > recognizer -> similarity(gesture3, {currentStroke, secondStroke, thirdStroke})){
                        _lastGesture = _gesturemap[gesture2];
                    }
                    _lastGesture = _gesturemap[gesture3];
                }
            }else if((currentStroke.size() > 3) & !secondStroke.empty()){
                std::string gesture = recognizer->match({ currentStroke });
                std::string gesture2 = recognizer -> match({ currentStroke, secondStroke });
                
                if(recognizer -> similarity(gesture,{ currentStroke }) > recognizer -> similarity(gesture2, {currentStroke, secondStroke})){
                    _lastGesture = _gesturemap[gesture];
                }else{
                    _lastGesture = _gesturemap[gesture2];
                }
            }else if (currentStroke.size() > 3){
                std::string gesture = recognizer->match({ currentStroke });
                _lastGesture = _gesturemap[gesture];
            }
            _timeSinceCast = 0;
        }
        catch (std::out_of_range) {
            // Not a recognized gesture... probably
            _lastGesture = SpellType::NONE;
        }

        if (_lastGesture != SpellType::NONE) {
            _isAiming = true;
        }

        //CULog("%s", gesture.c_str());
    }
    else {
        // Register tap as pickup, to be changed using buttons?
        _pickupPressed = true;
    }
//    CULog("First Stroke points:");
//        for (const auto& p : currentStroke) {
//            CULog("{%f, %f},", p.x, p.y);
//        }
//    CULog("Second Stroke points:");
//        for (const auto& p : secondStroke) {
//            CULog("{%f, %f},", p.x, p.y);
//        }
//    CULog("Third Stroke points:");
//        for (const auto& p : thirdStroke) {
//            CULog("{%f, %f},", p.x, p.y);
//        }

    // Clear points
    thirdStroke = secondStroke;
    secondStroke = currentStroke;
    currentStroke.clear();
}

void InputController::processDrag(const cugl::Vec2 pos) {
    if (_isAiming) {
        _spellAngle = pos - _touchStart;
    }
    else {
        currentStroke.push_back(pos);
    }
}

/**
 * Callback for the beginning of a touch event
 *
 * @param event The associated event
 * @param focus    Whether the listener currently has focus
 */
void InputController::touchBeganCB(const TouchEvent& event, bool focus) {
    _touchDown = true;
    //CULog("Touch began %lld", event.touch);

    _touchDown = true;
    Vec2 pos = event.position;
    Zone zone = getZone(pos);
    pos = touch2Screen(pos);
    
    CULog("Touch pos world: (%.2f, %.2f), zone: %d", pos.x, pos.y, (int)zone);
    CULog("lzone: (%.2f,%.2f,%.2f,%.2f)", _lzone.origin.x, _lzone.origin.y, _lzone.size.width, _lzone.size.height);
    CULog("rzone: (%.2f,%.2f,%.2f,%.2f)", _rzone.origin.x, _rzone.origin.y, _rzone.size.width, _rzone.size.height);
    switch (zone){
        case Zone::LEFT:
            // Only process if no touch in zone
            if (_ltouch.touchids.empty()) {
                // Left is the floating joystick
                _ltouch.position = event.position;
                _ltouch.timestamp.mark();
                _ltouch.touchids.insert(event.touch);

                _joycenter = pos;
                //_joycenter.y += JSTICK_OFFSET;
            }
            break;
        case Zone::RIGHT:
            // For now if pressed on right side attempt pickup, need to change to button
            _touchDown = true;
            _touchStart = pos;

            currentStroke.clear();
            currentStroke.push_back(pos);
            _rtouch.touchids.insert(event.touch);
            break;
        case Zone::MAIN:
            //not possible hopefully lol
            break;
        default:
            break;
    }
    
}

/**
 * Callback for the end of a touch event
 *
 * @param event The associated event
 * @param focus    Whether the listener currently has focus
 */
void InputController::touchEndCB(const TouchEvent& event, bool focus) {
    _touchDown = false;
    // Reset all keys that might have been set
    Vec2 pos = touch2Screen(event.position);
    Zone zone = getZone(pos);

    if (_ltouch.touchids.find(event.touch) != _ltouch.touchids.end()) {
        _joystick = false;
        _ltouch.touchids.clear();
        _move = Vec2::ZERO;
    } else if (_rtouch.touchids.find(event.touch) != _rtouch.touchids.end()) {
        _rtouch.touchids.clear();
        processGestureEnd(pos);
    } else if (zone == Zone::MAIN) {
    }
}

/**
 * Callback for a touch moved event.
 *
 * @param event The associated event
 * @param previous The previous position of the touch
 * @param focus    Whether the listener currently has focus
 */
void InputController::touchMotionCB(const TouchEvent& event, const Vec2& previous, bool focus) {
    Vec2 pos = touch2Screen(event.position);
    // Left zone is all that matters rn. 
    if (_ltouch.touchids.find(event.touch) != _ltouch.touchids.end()) {
        processJoystick(pos);
    }
    else if (_rtouch.touchids.find(event.touch) != _rtouch.touchids.end()) {
        processDrag(pos);
    }
}

void InputController::buttonDownCB(const cugl::MouseEvent& event, Uint8 clicks, bool focus) {
    _touchDown = true;
    CULog("Mouse press");

    Vec2 pos = mouse2Screen(event.position);
    Zone zone = getZone(pos);

    switch (zone) {
    case Zone::LEFT:
        // Left is the floating joystick
        _mouseDownZone = Zone::LEFT;

        _joycenter = pos;
        break;
    case Zone::RIGHT:
        // For now if pressed on right side attempt pickup, need to change to button
        _touchDown = true;
        _touchStart = pos;

        currentStroke.clear();
        currentStroke.push_back(pos);
        //_rtouch.touchids.insert(event.touch);
        _mouseDownZone = Zone::RIGHT;
        break;
    case Zone::MAIN:
        //not possible hopefully lol
        _mouseDownZone = Zone::MAIN;

        break;
    default:
        _mouseDownZone = Zone::UNDEFINED;
        break;
    }
}

void InputController::buttonUpCB(const cugl::MouseEvent& event, Uint8 clicks, bool focus) {
    _touchDown = false;
    // Reset all keys that might have been set
    Vec2 pos = mouse2Screen(event.position);
    Zone zone = getZone(pos);

    if (_mouseDownZone == Zone::LEFT) {
        //CULog("(%.2f, %.2f) (%.2f, %.2f)", _joycenter.x, _joycenter.y, pos.x, pos.y);
        _ltouch.touchids.clear();
        _move = Vec2::ZERO;
        _joystick = false;
    }
    else if (_mouseDownZone == Zone::RIGHT) {
        _rtouch.touchids.clear();
        processGestureEnd(pos);
    }
    else if (zone == Zone::MAIN) {
    }
    _mouseDownZone = Zone::UNDEFINED;
}

void InputController::motionCB(const cugl::MouseEvent& event, const cugl::Vec2 previous, bool focus) {
    Vec2 pos = mouse2Screen(event.position);

    if (_mouseDownZone == Zone::LEFT) {
        processJoystick(pos);
    }
    else if (_mouseDownZone == Zone::RIGHT) {
        processDrag(pos);
    }
}


void InputController::activate() {
#ifdef CU_TOUCH_SCREEN
    Touchscreen* touch = Input::get<Touchscreen>();
    if (touch) {
        touch->addBeginListener(LISTENER_KEY, [this](const cugl::TouchEvent& event, bool focus) {
            this->touchBeganCB(event, focus);
        });
        touch->addEndListener(LISTENER_KEY, [this](const cugl::TouchEvent& event, bool focus) {
            this->touchEndCB(event, focus);
        });
        touch->addMotionListener(LISTENER_KEY, [this](const cugl::TouchEvent& event,const Vec2 previous, bool focus) {
            this->touchMotionCB(event,previous, focus);
        });
        _active = true;
    } else {
        _active = false;
    }
#else
    _active = Input::activate<Keyboard>();
    Mouse* mouse = Input::get<Mouse>();
    if (_active && mouse) {
        mouse->setPointerAwareness(Mouse::PointerAwareness::DRAG);
        _mouseKey = mouse->acquireKey();

        mouse->addPressListener(_mouseKey, [=, this](const cugl::MouseEvent& event, Uint8 clicks, bool focus) {
            this->buttonDownCB(event, clicks, focus);
        });
        mouse->addReleaseListener(_mouseKey, [=, this](const cugl::MouseEvent& event, Uint8 clicks, bool focus) {
            this->buttonUpCB(event, clicks, focus);
        });
        mouse->addDragListener(_mouseKey, [=, this](const cugl::MouseEvent& event, const Vec2 previous, bool focus) {
            this->motionCB(event, previous, focus);
        });
        _active = true;

        CULog("Added listeners!");
    }
    else {
        _active = false;
    }
#endif
}

void InputController::deactivate() {
#ifndef CU_TOUCH_SCREEN
    Mouse* mouse = Input::get<Mouse>();
    mouse->removePressListener(LISTENER_KEY);
    mouse->removeReleaseListener(LISTENER_KEY);
    mouse->removeDragListener(LISTENER_KEY);
#else
    Touchscreen* touch = Input::get<Touchscreen>();
    touch->removeBeginListener(LISTENER_KEY);
    touch->removeEndListener(LISTENER_KEY);
#endif
    _active = false;
}
