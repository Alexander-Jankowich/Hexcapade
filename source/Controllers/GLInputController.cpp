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
//#define DOUBLE_CLICK    400
std::vector<std::vector<Vec2>> DEATH_GESTURE_1 = {
    {
        {1058.320557, 465.343506},
        {1058.931274, 456.183228},
        {1058.931274, 438.473267},
        {1058.320557, 406.106812},
        {1050.992310, 357.862549},
        {1045.496094, 321.832031},
        {1041.832031, 292.519043},
        {1038.778564, 268.091553},
        {1038.167969, 242.442749},
        {1038.167969, 215.572510},
        {1038.167969, 192.366455},
        {1038.167969, 175.877930},
        {1040.610718, 163.664185},
        {1043.664062, 153.893188},
        {1044.885498, 145.954224},
        {1046.717529, 141.068726},
        {1047.938965, 138.015259},
        {1047.938965, 136.793945},
    },
    {
        {955.725098, 327.938965},
        {964.274841, 308.396973},
        {973.435059, 295.572510},
        {985.648865, 278.473267},
        {996.030518, 260.763306},
        {1005.801514, 244.274780},
        {1014.961853, 229.618286},
        {1022.900757, 216.793945},
        {1030.229004, 205.801514},
        {1034.503784, 199.694702},
        {1035.114502, 197.862549},
        {1035.114502, 199.694702},
        {1035.114502, 203.358765},
        {1035.114502, 203.358765},
    },
    {
        {959.389343, 452.519043},
        {956.946533, 441.526733},
        {956.946533, 415.267212},
        {956.335938, 399.389282},
        {955.725098, 382.900757},
        {955.725098, 351.145020},
        {955.725098, 323.053467},
        {955.114502, 294.351196},
        {952.671692, 263.816772},
        {950.839722, 232.671753},
        {949.618286, 205.190918},
        {948.396912, 185.038208},
        {947.786194, 185.038208},
        {947.786194, 190.534424},
        {947.786194, 194.809204},
        {947.786194, 199.083984},
        {947.786194, 203.969482},
        {947.786194, 203.969482}
    }
};
std::vector<std::vector<Vec2>> DEATH_GESTURE = {
    {
        {1152.366333, 423.816711},
        {1144.427490, 409.770935},
        {1144.427490, 400.610657},
        {1144.427490, 384.122070},
        {1144.427490, 358.473206},
        {1144.427490, 323.664062},
        {1144.427490, 288.244263},
        {1145.038086, 255.877808},
        {1147.480835, 225.954224},
        {1149.312988, 199.083923},
        {1150.534424, 176.488525},
        {1150.534424, 159.389282},
        {1150.534424, 147.175537},
        {1150.534424, 139.236572},
        {1150.534424, 134.351074},
        {1150.534424, 132.519043},
        {1149.923706, 130.687012},
        {1148.702271, 130.687012},
        {1147.480835, 130.687012},
        {1146.870239, 130.687012},
        {1146.870239, 131.908386}
    },
    {
        {997.862549, 323.664062},
        {1012.519043, 303.511414},
        {1022.290039, 292.519043},
        {1035.114502, 276.030457},
        {1052.213745, 256.488525},
        {1071.145020, 236.946533},
        {1086.412231, 220.457947},
        {1098.625977, 207.022888},
        {1109.007568, 196.030518},
        {1118.778564, 186.259521},
        {1126.717529, 177.709900},
        {1132.213745, 171.603027},
        {1135.877808, 166.106812},
        {1138.320679, 161.832031},
        {1140.763306, 156.946533},
        {1141.984741, 153.282410},
        {1141.984741, 152.671753},
        {1141.984741, 155.114502},
        {1141.984741, 155.114502},
    },
    {
        {1008.854980, 407.938843},
        {1010.076294, 392.671692},
        {1010.076294, 373.129700},
        {1009.465576, 339.541931},
        {1005.801514, 301.068665},
        {1003.358765, 266.259521},
        {1002.137451, 235.725159},
        {1001.526733, 209.465637},
        {1001.526733, 186.870178},
        {1001.526733, 166.717529},
        {1001.526733, 151.450317},
        {1001.526733, 141.068665},
        {1001.526733, 136.183167},
        {1001.526733, 134.351074},
        {1001.526733, 133.740417},
        {1000.916016, 133.740417},
        {1000.305298, 133.740417},
        {1000.305298, 136.183167},
        {1000.305298, 136.183167}
    }
};
std::vector<std::vector<Vec2>> X_GESTURE = {
    
        {
            {792.040039, 255.920410},
            {793.632080, 253.134277},
            {796.418213, 241.989990},
            {796.418213, 239.203979},
            {796.418213, 229.651733},
            {796.816162, 195.820923},
            {799.204102, 179.104492},
            {801.592285, 163.184082},
            {804.378418, 150.447754},
            {806.368408, 141.691528},
            {807.562500, 137.711426},
            {807.960449, 137.711426},
            {807.960449, 138.905518},
            {807.960449, 140.497437},
            {807.960449, 142.487549},
            {807.960449, 145.671631},
            {811.144531, 154.029907},
            {818.308594, 171.940308},
            {828.258789, 195.422852},
            {837.811035, 215.721436},
            {846.169434, 230.845825},
            {852.935547, 239.601929},
            {855.721680, 241.989990},
            {856.517578, 241.989990},
            {856.517578, 241.592041},
            {856.517578, 240.796021},
            {856.517578, 239.601929},
            {856.517578, 238.805908},
            {856.517578, 238.407959},
            {856.517578, 236.815918},
            {856.517578, 236.019897},
            {856.915527, 233.233765},
            {861.293701, 224.875610},
            {866.865967, 207.761230},
            {872.835938, 188.656738},
            {877.612061, 172.736328},
            {881.990234, 160.398071},
            {885.174316, 151.641846},
            {887.562500, 147.661621},
            {889.154297, 145.671631},
            {889.950439, 144.875610},
            {890.348389, 144.875610},
            {890.348389, 145.273682},
            {890.348389, 148.059692},
            {890.348389, 152.437744},
            {893.532471, 163.582031},
            {902.686768, 189.054688},
            {914.626953, 217.711426},
            {921.791260, 234.825928},
            {924.577393, 241.592041},
            {925.373291, 243.582031},
            {926.567383, 243.980103},
            {927.363281, 243.980103},
            {928.159424, 243.980103},
            {929.353516, 244.776123},
            {930.149414, 245.572144}
        }
    
};


std::vector<std::vector<Vec2>> ARROW_GESTURE = {
    {
        {2033.129883, 749.465759},
        {2055.725342, 787.938965},
        {2091.145020, 828.855042},
        {2128.396973, 863.664185},
        {2155.267334, 888.702332},
        {2171.145020, 898.473328},
        {2176.641357, 899.084045},
        {2178.473389, 899.084045},
        {2179.083984, 899.084045},
        {2179.083984, 897.862671},
        {2179.694824, 896.641296},
        {2180.916016, 895.419922},
        {2182.748047, 894.809204},
        {2188.244385, 891.755798},
        {2204.122070, 883.206177},
        {2231.603027, 862.442810},
        {2260.305420, 833.740540},
        {2283.511719, 805.038208},
        {2298.167969, 783.053467},
        {2303.664307, 773.282532},
        {2304.274902, 772.061157},
        {2304.274902, 773.893188}
    },
    {
        {2183.969482, 864.885620},
        {2182.137451, 834.351196},
        {2182.137451, 812.366516},
        {2182.137451, 783.053467},
        {2180.916016, 659.694763},
        {2190.687256, 605.954285},
        {2201.679443, 565.038269},
        {2209.618408, 543.664185},
        {2210.839844, 536.946655},
        {2210.839844, 536.335938},
        {2207.175781, 537.557373}
    }
};

std::vector<std::vector<Vec2>> ARROW_GESTURE_2 = {
    {
        {870.229004, 2021.374023},
        {881.832031, 2017.709961},
        {903.206116, 2042.137451},
        {934.961792, 2076.946533},
        {967.938965, 2111.145020},
        {994.198425, 2135.572510},
        {1007.633545, 2141.679443},
        {1010.687012, 2141.068604},
        {1012.519043, 2141.068604},
        {1013.129761, 2141.068604},
        {1013.129761, 2140.458008},
        {1013.129761, 2139.236572},
        {1013.129761, 2138.015137},
        {1013.740479, 2135.572510},
        {1018.625977, 2129.465576},
        {1037.557251, 2112.977051},
        {1071.145020, 2085.496094},
        {1107.175537, 2056.183105},
        {1138.320679, 2031.755737},
        {1159.083984, 2015.877930},
        {1170.687012, 2007.328247},
        {1175.572510, 2003.053467},
        {1176.183228, 2002.442749}
    },
    {
        {1012.519043, 2111.145020},
        {1011.297729, 2107.480957},
        {1009.465637, 2073.893066},
        {1009.465637, 2061.068848},
        {1008.244263, 2015.267212},
        {1007.022827, 1951.755737},
        {1007.022827, 1891.297729},
        {1009.465637, 1837.557373},
        {1013.129761, 1792.977051},
        {1013.129761, 1768.549561},
        {1013.129761, 1762.442749},
        {1012.519043, 1762.442749},
        {1011.908325, 1762.442749},
        {1011.908325, 1762.442749}
    }
};

std::vector<std::vector<Vec2>> ARROW_GESTURE_3 =
{
    {
        {1027.786255, 866.530334},
        {1035.114502, 872.026489},
        {1057.099243, 892.179199},
        {1086.412231, 921.492126},
        {1109.007568, 947.140991},
        {1135.877808, 985.614258},
        {1138.320679, 990.499817},
        {1137.709961, 990.499817},
        {1137.709961, 989.889099},
        {1137.709961, 988.057007},
        {1137.709961, 986.835632},
        {1137.709961, 986.224976},
        {1138.320679, 985.614258},
        {1139.541992, 985.614258},
        {1143.816772, 983.782166},
        {1159.083984, 976.453979},
        {1185.954102, 959.965393},
        {1215.267090, 938.591370},
        {1240.916016, 917.828003},
        {1259.236572, 901.339478},
        {1270.229004, 890.957764},
        {1273.893188, 888.515076},
        {1274.503906, 888.515076}
    },
    {
        {1145.038086, 988.057007},
        {1140.152588, 972.179199},
        {1140.763306, 942.866211},
        {1140.763306, 930.652466},
        {1141.984741, 886.682983},
        {1147.480835, 824.392883},
        {1157.862549, 771.263184},
        {1171.297729, 724.240234},
        {1180.458008, 693.095215},
        {1184.732788, 673.553223},
        {1184.122070, 659.507446},
        {1178.015259, 658.896729},
        {1175.572510, 658.896729},
        {1175.572510, 658.896729}
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

std::vector<std::vector<Vec2>> HEAL_GESTURE_2 = {
    {
        {1052.213745, 230.228973},
        {1058.931274, 237.557220},
        {1077.862549, 254.656494},
        {1099.847412, 277.251892},
        {1124.885498, 306.564850},
        {1151.755615, 337.709900},
        {1177.404541, 364.580139},
        {1198.778564, 385.954193},
        {1212.824463, 396.946503},
        {1225.648804, 401.221344},
        {1229.312988, 403.053406},
        {1229.312988, 403.053406},
    },
    {
        {1033.282349, 327.938904},
        {1043.053467, 329.160278},
        {1055.267090, 340.152649},
        {1076.641235, 359.083954},
        {1102.290039, 382.290039},
        {1124.274780, 401.221344},
        {1138.931274, 412.213684},
        {1147.480835, 417.709900},
        {1154.809204, 421.374023},
        {1160.305298, 424.427429},
        {1167.022949, 427.480865},
        {1173.129761, 429.923615},
        {1181.679321, 432.366394},
        {1181.679321, 432.366394}
    },
    {
        {1027.175537, 436.641174},
        {1033.893066, 426.870178},
        {1033.282349, 422.595398},
        {1033.282349, 404.885498},
        {1033.282349, 370.076324},
        {1033.282349, 319.999939},
        {1034.503784, 271.755737},
        {1038.167969, 229.618301},
        {1041.832031, 194.198502},
        {1044.885498, 167.938934},
        {1046.717529, 149.007660},
        {1047.938965, 130.687042},
        {1047.938965, 131.908417},
        {1047.938965, 134.961823},
        {1048.549561, 137.404572},
        {1048.549561, 137.404572},
    }
};
std::vector<std::vector<Vec2>> FREEZE_GESTURE = {
    {
        {1035.725220, 892.234314},
        {1025.343506, 897.119751},
        {1006.412231, 908.112122},
        {980.763367, 924.600647},
        {949.007568, 944.142639},
        {920.916016, 960.631165},
        {900.152710, 974.066284},
        {890.381653, 981.394531},
        {888.549683, 985.058655},
        {888.549683, 985.669373},
        {888.549683, 983.226624},
        {894.656494, 977.119751},
        {900.763306, 971.012878},
        {906.870178, 960.631165},
        {909.312988, 930.096802},
        {909.312988, 868.417480},
        {916.030518, 795.135010},
        {928.854980, 740.173218},
        {939.847290, 710.249512},
        {944.732788, 694.371704},
        {945.343506, 684.600647},
        {945.343506, 674.829712},
        {945.343506, 667.501465},
        {945.343506, 663.226685},
        {945.343506, 661.394592},
        {944.732788, 660.173218},
        {943.511475, 659.562500},
        {941.068665, 659.562500},
        {938.625977, 659.562500},
        {935.572449, 659.562500},
        {931.908325, 659.562500},
        {927.633545, 659.562500},
        {920.916016, 661.394592},
        {909.312988, 668.112183},
        {889.160278, 680.936523},
        {861.068726, 703.531982},
        {834.198425, 724.906006},
        {812.824402, 740.173218},
        {802.442749, 747.501404},
        {801.221375, 748.722778},
        {807.328186, 748.722778},
        {820.152588, 747.501404},
        {827.480896, 746.280090}
    }
};

std::vector<std::vector<Vec2>> WATER_GESTURE = {
    {
        {975.877869, 277.862610},
        {988.702271, 278.473267},
        {1033.282349, 279.694702},
        {1068.702271, 279.694702},
        {1105.954102, 279.694702},
        {1142.595337, 278.473267},
        {1178.015259, 278.473267},
        {1212.213745, 278.473267},
        {1239.694702, 278.473267},
        {1258.015259, 279.694702},
        {1264.122070, 281.526733},
        {1264.732788, 282.137390},
        {1263.511475, 282.137390},
        {1262.290039, 282.137390},
        {1260.458008, 282.748108},
        {1258.015259, 283.969482},
        {1256.183228, 285.190857},
        {1252.519043, 287.022888},
        {1249.465576, 288.854919},
        {1245.190796, 290.687012},
        {1240.916016, 293.740479},
        {1234.198486, 298.015259},
        {1223.206055, 305.343567},
        {1207.328125, 317.557312},
        {1189.618286, 330.992371},
        {1171.297729, 342.595398},
        {1157.251953, 350.534424},
        {1148.702271, 356.641235},
        {1147.480835, 359.083984},
        {1147.480835, 360.305359},
        {1146.870239, 360.305359},
        {1146.259521, 360.305359},
        {1145.648926, 360.305359},
        {1145.648926, 357.862610},
        {1145.648926, 349.312988},
        {1149.923706, 335.877930},
        {1151.755615, 310.229004},
        {1151.755615, 267.480957},
        {1149.312988, 218.625977},
        {1143.206055, 172.824463},
        {1139.541992, 137.404602},
        {1138.931274, 117.862610},
        {1138.931274, 111.755798},
        {1138.931274, 111.145050},
        {1138.931274, 111.755798},
        {1138.931274, 115.419891},
        {1138.931274, 116.641205}
    }
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

/** The portion of the screen used for the top zone */
#define TOP_ZONE        0.20f
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
_cardDismissTap(false),
_touchID(-1),
_mouseKey(0),
_joystick(false),
_pickupPressed(false),
_depositPressed(false),
_inventoryReleased(false),
_inventoryAnchor(Vec2::ZERO),
_inventoryTouchPos(Vec2::ZERO),
_inventoryHotkeyIndex(-1),
_resetPressed(false),
_lastGesture(SpellType::NONE),
_isAiming(false),
_releaseSpell(false),
_timeSinceCast(0),
_spellAngle(Vec2::ZERO),
_timeSinceLastStroke(0),
_curStrokes(0),
_gestureRecognized(false)
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
    recognizer -> addGesture("Death", DEATH_GESTURE);
    recognizer -> addGesture("Death", DEATH_GESTURE_1);
    recognizer -> addGesture("Earth", X_GESTURE);
    recognizer -> addGesture("Fire", ARROW_GESTURE);
    recognizer -> addGesture("Fire", ARROW_GESTURE_2);
    recognizer -> addGesture("Fire", ARROW_GESTURE_3);
    recognizer -> addGesture("Freeze", FREEZE_GESTURE);
    recognizer -> addGesture("Heal", HEAL_GESTURE);
    recognizer -> addGesture("Heal", HEAL_GESTURE_2);
    recognizer -> addGesture("Water", WATER_GESTURE);
    

    recognizer->set_accuracy(0.83f);

    // Update the gesturemap with more spells later on
    _gesturemap = {
        {"Fire", SpellType::FIREBALL},
        {"Freeze", SpellType::FREEZE},
        {"Heal", SpellType::HEAL },
        {"Death", SpellType::DEATH},
        {"Earth", SpellType::EARTH},
        {"Water", SpellType::WATER}
    };

    _joyradius = values->get("joystick")->getInt("radius");
    _deadzone = values->get("joystick")->getInt("dead_zone");

#ifndef CU_TOUCH_SCREEN
    _sbounds = bounds;

    Size display = Application::get()->getDisplaySize();
    _tbounds = Rect(0,0,display.width,display.height);
        
    createZones();
    clearTouchInstance(_ltouch);
    clearTouchInstance(_rtouch);
    clearTouchInstance(_itouch);
    clearTouchInstance(_mtouch);
#else
    Size display = Application::get()->getDrawableSize();
    _tbounds = Rect(0,0,display.width,display.height);
    _sbounds = _tbounds;
    createZones();
    clearTouchInstance(_ltouch);
    clearTouchInstance(_rtouch);
    clearTouchInstance(_itouch);
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
    }

    // Update time since last gesture movement
    _timeSinceLastStroke += dt;
    if (_curStrokes > 0 && _timeSinceLastStroke > 1 && currentStroke.size() == 0) {
        // Too long has passed since the last stroke, clear gesture
        CULog("Spell drawing took too long!");
        thirdStroke.clear();
        secondStroke.clear();
        currentStroke.clear();
        _curStrokes = 0;
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
    _depositPressed = false;
    _inventoryHotkeyIndex = -1;
    _resetPressed = false;

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

        bool deposit = keys->keyDown(KeyCode::F);
        static bool depositDown = false;

        _depositPressed = deposit && !depositDown;
        depositDown = deposit;

        KeyCode slotKeys[5] = {
            KeyCode::NUM_1,
            KeyCode::NUM_2,
            KeyCode::NUM_3,
            KeyCode::NUM_4,
            KeyCode::NUM_5
        };
        static bool slotDown[5] = { false, false, false, false, false };
        for (int i = 0; i < 5; ++i) {
            bool slotPressed = keys->keyDown(slotKeys[i]);
            if (slotPressed && !slotDown[i]) {
                _inventoryHotkeyIndex = i;
            }
            slotDown[i] = slotPressed;
        }


        // Keyboard inputs for spells
        bool space = keys->keyDown(KeyCode::SPACE);
        bool shift = keys->keyDown(KeyCode::LEFT_SHIFT);
        static bool spacePressed = false;
        static bool shiftPressed = false;
        if (space && !spacePressed) {
            _lastGesture = SpellType::FIREBALL;
            _spellAngle.setZero();
            _isAiming = true;
            _gestureRecognized = true;
            _curStrokes = 0;
        }
        spacePressed = space;

        if (shift && !shiftPressed) {
            _lastGesture = SpellType::FREEZE;
            _spellAngle.setZero();
            _isAiming = true;
            _gestureRecognized = true;
            _curStrokes = 0;
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
    _tzone = _sbounds;
    _tzone.size.height *= TOP_ZONE;

    _lzone = _sbounds;
    _lzone.size.width *= LEFT_ZONE;
    _lzone.origin.y += _tzone.size.height;
    _lzone.size.height -= _tzone.size.height;

    _rzone = _sbounds;
    _rzone.size.width *= RIGHT_ZONE;
    _rzone.origin.x = _sbounds.origin.x+_sbounds.size.width-_rzone.size.width;
    _rzone.size.height -= _tzone.size.height;
    _rzone.origin.y += _tzone.size.height;
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
    if (_tzone.contains(pos)) {
        return Zone::TOP;
    } else if (_lzone.contains(pos)) {
        return Zone::LEFT;
    } else if (_rzone.contains(pos)) {
        return Zone::RIGHT;
    } else if (_sbounds.contains(pos)) {
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
    return _screenToWorld(pos) - _cameraOffset;
}

Vec2 InputController::mouse2Screen(const Vec2 pos) const {
    return _screenToWorld(pos) - _cameraOffset;
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

        // Move joystick center in excess direction
        //_joycenter += excess;

        // Move the joystick to be within the joystick radius
        _joystickpos -= excess;
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
    }
    else {
        _joystick = false;
        _move = Vec2(0,0);
    }
}

void InputController::processGestureEnd(const cugl::Vec2 pos) {
    if (_isAiming) {
        _isAiming = false;
        _releaseSpell = true;
        _spellAngle = pos - _touchStart;
        //CULog("Angle: %.2f %.2f", _spellAngle.x, _spellAngle.y);
        return;
    }
    CULog("First Stroke points:");
    for (const auto& p : currentStroke) {
        CULog("{%f, %f},", p.x, p.y);
    }
    CULog("Second Stroke points:");
    for (const auto& p : secondStroke) {
        CULog("{%f, %f},", p.x, p.y);
    }
    CULog("Third Stroke points:");
    for (const auto& p : thirdStroke) {
        CULog("{%f, %f},", p.x, p.y);
    }
    _timeSinceLastStroke = 0;
    _curStrokes++;
    if (_curStrokes > 3) {
        _curStrokes -= 3;
    }

    _spellAngle.setZero();
    currentStroke.push_back(pos);

    // Only check if it's larger than a certain size
    if (currentStroke.size() > 5) {
        try {
            // Check all the strokes currently on the screen!
            if (_curStrokes == 1) {
                _lastGesture = _gesturemap[recognizer->match({ currentStroke })];
            }
            else if (_curStrokes == 2) {
                _lastGesture = _gesturemap[recognizer->match({ currentStroke, secondStroke })];
            }
            else if (_curStrokes == 3) {
                _lastGesture = _gesturemap[recognizer->match({ currentStroke, secondStroke, thirdStroke })];
            }
            else {
                CULog("Error! Number of strokes recorded is more than 3???");
            }
            _timeSinceCast = 0;
        }
        catch (std::out_of_range) {
            // Not a recognized gesture... probably
            _lastGesture = SpellType::NONE;
        }

        if (_lastGesture != SpellType::NONE) {
            CULog("Correct spell!");
            _isAiming = true;
            thirdStroke.clear();
            secondStroke.clear();
            currentStroke.clear();
            _gestureRecognized = true;
            _curStrokes = 0;
        }
        else if (_curStrokes >= 3) {
            CULog("Not Correct spell! --------------------------------");
            // Clear all strokes if number is greater than 3
            thirdStroke.clear();
            secondStroke.clear();
            currentStroke.clear();
            _gestureRecognized = false;
        }
        else {
            CULog("Still searching...");
            thirdStroke = secondStroke;
            secondStroke = currentStroke;
            currentStroke.clear();
            _gestureRecognized = false;
        }

        //CULog("%s", gesture.c_str());
    }
    else if (currentStroke[0].distance(currentStroke[currentStroke.size() - 1]) < 10) {
        CULog("Pickup, clear gesture");
        currentStroke.clear();
        secondStroke.clear();
        thirdStroke.clear();
        _curStrokes = 0;
        _pickupPressed = true;
    }
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
    _cardDismissTap = true;
    pos = touch2Screen(pos);
    
    switch (zone){
    case Zone::TOP:
        if (_itouch.touchids.empty()) {
            _inventoryAnchor = pos;
            _inventoryTouchPos = pos;
            _itouch.position = pos;
            _itouch.timestamp.mark();
            _itouch.touchids.insert(event.touch);
        }
        break;
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
        if (!_isAiming) {
            _gestureRecognized = false;
            if (_curStrokes >= 3) {
                _curStrokes = 0;
            }
        }
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
    Vec2 pos = event.position;
    Zone zone = getZone(pos);
    pos = touch2Screen(pos);
   

    if (_ltouch.touchids.find(event.touch) != _ltouch.touchids.end()) {
        _joystick = false;
        _ltouch.touchids.clear();
        _move = Vec2::ZERO;
    } else if (_rtouch.touchids.find(event.touch) != _rtouch.touchids.end()) {
        _rtouch.touchids.clear();
        processGestureEnd(pos);
    } else if (_itouch.touchids.find(event.touch) != _itouch.touchids.end()) {
        _itouch.touchids.clear();
        _inventoryReleased = true;
        _inventoryTouchPos = pos;
    } else if (zone == Zone::TOP) {
        _inventoryReleased = true;
        _inventoryTouchPos = pos;
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
    else if (_itouch.touchids.find(event.touch) != _itouch.touchids.end()) {
        _inventoryTouchPos = pos;
    }
}

void InputController::buttonDownCB(const cugl::MouseEvent& event, Uint8 clicks, bool focus) {
    _touchDown = true;
    //CULog("Mouse press");

    Vec2 pos = mouse2Screen(event.position);
    Zone zone = getZone(pos);
    _cardDismissTap = true;

    switch (zone) {
    case Zone::TOP:
        _mouseDownZone = Zone::TOP;
        break;
    case Zone::LEFT: // Do not use joystick for mouse input
    case Zone::RIGHT:
        // For now if pressed on right side attempt pickup, need to change to button
        _touchDown = true;
        _touchStart = pos;

        currentStroke.clear();
        currentStroke.push_back(pos);
        //_rtouch.touchids.insert(event.touch);
        _mouseDownZone = Zone::RIGHT;
        if (!_isAiming) {
            _gestureRecognized = false;
            if (_curStrokes >= 3) {
                _curStrokes = 0;
            }
        }
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

    if (_mouseDownZone == Zone::LEFT || _mouseDownZone == Zone::RIGHT) {
        _rtouch.touchids.clear();
        processGestureEnd(pos);
    }
    else if (_mouseDownZone == Zone::TOP) {
    }
    else if (zone == Zone::MAIN) {
    }
    _mouseDownZone = Zone::UNDEFINED;
}

void InputController::motionCB(const cugl::MouseEvent& event, const cugl::Vec2 previous, bool focus) {
    Vec2 pos = mouse2Screen(event.position);

    if (_mouseDownZone == Zone::LEFT || _mouseDownZone == Zone::RIGHT) {
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
