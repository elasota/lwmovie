/* The contents of this file was automatically generated by dump_modes.c
   with arguments: 48000 960
   It contains static definitions for some pre-defined modes. */
#include "modes.h"
#include "rate.h"

#ifndef DEF_WINDOW120
#define DEF_WINDOW120
static const opus_val16 window120[120] = {
6.7286966e-05f, 0.00060551348f, 0.0016815970f, 0.0032947962f, 0.0054439943f,
0.0081276923f, 0.011344001f, 0.015090633f, 0.019364886f, 0.024163635f,
0.029483315f, 0.035319905f, 0.041668911f, 0.048525347f, 0.055883718f,
0.063737999f, 0.072081616f, 0.080907428f, 0.090207705f, 0.099974111f,
0.11019769f, 0.12086883f, 0.13197729f, 0.14351214f, 0.15546177f,
0.16781389f, 0.18055550f, 0.19367290f, 0.20715171f, 0.22097682f,
0.23513243f, 0.24960208f, 0.26436860f, 0.27941419f, 0.29472040f,
0.31026818f, 0.32603788f, 0.34200931f, 0.35816177f, 0.37447407f,
0.39092462f, 0.40749142f, 0.42415215f, 0.44088423f, 0.45766484f,
0.47447104f, 0.49127978f, 0.50806798f, 0.52481261f, 0.54149077f,
0.55807973f, 0.57455701f, 0.59090049f, 0.60708841f, 0.62309951f,
0.63891306f, 0.65450896f, 0.66986776f, 0.68497077f, 0.69980010f,
0.71433873f, 0.72857055f, 0.74248043f, 0.75605424f, 0.76927895f,
0.78214257f, 0.79463430f, 0.80674445f, 0.81846456f, 0.82978733f,
0.84070669f, 0.85121779f, 0.86131698f, 0.87100183f, 0.88027111f,
0.88912479f, 0.89756398f, 0.90559094f, 0.91320904f, 0.92042270f,
0.92723738f, 0.93365955f, 0.93969656f, 0.94535671f, 0.95064907f,
0.95558353f, 0.96017067f, 0.96442171f, 0.96834849f, 0.97196334f,
0.97527906f, 0.97830883f, 0.98106616f, 0.98356480f, 0.98581869f,
0.98784191f, 0.98964856f, 0.99125274f, 0.99266849f, 0.99390969f,
0.99499004f, 0.99592297f, 0.99672162f, 0.99739874f, 0.99796667f,
0.99843728f, 0.99882195f, 0.99913147f, 0.99937606f, 0.99956527f,
0.99970802f, 0.99981248f, 0.99988613f, 0.99993565f, 0.99996697f,
0.99998518f, 0.99999457f, 0.99999859f, 0.99999982f, 1.0000000f,
};
#endif

#ifndef DEF_LOGN400
#define DEF_LOGN400
static const opus_int16 logN400[21] = {
0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 8, 8, 16, 16, 16, 21, 21, 24, 29, 34, 36, };
#endif

#ifndef DEF_PULSE_CACHE50
#define DEF_PULSE_CACHE50
static const opus_int16 cache_index50[105] = {
-1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 41, 41, 41,
82, 82, 123, 164, 200, 222, 0, 0, 0, 0, 0, 0, 0, 0, 41,
41, 41, 41, 123, 123, 123, 164, 164, 240, 266, 283, 295, 41, 41, 41,
41, 41, 41, 41, 41, 123, 123, 123, 123, 240, 240, 240, 266, 266, 305,
318, 328, 336, 123, 123, 123, 123, 123, 123, 123, 123, 240, 240, 240, 240,
305, 305, 305, 318, 318, 343, 351, 358, 364, 240, 240, 240, 240, 240, 240,
240, 240, 305, 305, 305, 305, 343, 343, 343, 351, 351, 370, 376, 382, 387,
};
static const unsigned char cache_bits50[392] = {
40, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 40, 15, 23, 28,
31, 34, 36, 38, 39, 41, 42, 43, 44, 45, 46, 47, 47, 49, 50,
51, 52, 53, 54, 55, 55, 57, 58, 59, 60, 61, 62, 63, 63, 65,
66, 67, 68, 69, 70, 71, 71, 40, 20, 33, 41, 48, 53, 57, 61,
64, 66, 69, 71, 73, 75, 76, 78, 80, 82, 85, 87, 89, 91, 92,
94, 96, 98, 101, 103, 105, 107, 108, 110, 112, 114, 117, 119, 121, 123,
124, 126, 128, 40, 23, 39, 51, 60, 67, 73, 79, 83, 87, 91, 94,
97, 100, 102, 105, 107, 111, 115, 118, 121, 124, 126, 129, 131, 135, 139,
142, 145, 148, 150, 153, 155, 159, 163, 166, 169, 172, 174, 177, 179, 35,
28, 49, 65, 78, 89, 99, 107, 114, 120, 126, 132, 136, 141, 145, 149,
153, 159, 165, 171, 176, 180, 185, 189, 192, 199, 205, 211, 216, 220, 225,
229, 232, 239, 245, 251, 21, 33, 58, 79, 97, 112, 125, 137, 148, 157,
166, 174, 182, 189, 195, 201, 207, 217, 227, 235, 243, 251, 17, 35, 63,
86, 106, 123, 139, 152, 165, 177, 187, 197, 206, 214, 222, 230, 237, 250,
25, 31, 55, 75, 91, 105, 117, 128, 138, 146, 154, 161, 168, 174, 180,
185, 190, 200, 208, 215, 222, 229, 235, 240, 245, 255, 16, 36, 65, 89,
110, 128, 144, 159, 173, 185, 196, 207, 217, 226, 234, 242, 250, 11, 41,
74, 103, 128, 151, 172, 191, 209, 225, 241, 255, 9, 43, 79, 110, 138,
163, 186, 207, 227, 246, 12, 39, 71, 99, 123, 144, 164, 182, 198, 214,
228, 241, 253, 9, 44, 81, 113, 142, 168, 192, 214, 235, 255, 7, 49,
90, 127, 160, 191, 220, 247, 6, 51, 95, 134, 170, 203, 234, 7, 47,
87, 123, 155, 184, 212, 237, 6, 52, 97, 137, 174, 208, 240, 5, 57,
106, 151, 192, 231, 5, 59, 111, 158, 202, 243, 5, 55, 103, 147, 187,
224, 5, 60, 113, 161, 206, 248, 4, 65, 122, 175, 224, 4, 67, 127,
182, 234, };
static const unsigned char cache_caps50[168] = {
224, 224, 224, 224, 224, 224, 224, 224, 160, 160, 160, 160, 185, 185, 185,
178, 178, 168, 134, 61, 37, 224, 224, 224, 224, 224, 224, 224, 224, 240,
240, 240, 240, 207, 207, 207, 198, 198, 183, 144, 66, 40, 160, 160, 160,
160, 160, 160, 160, 160, 185, 185, 185, 185, 193, 193, 193, 183, 183, 172,
138, 64, 38, 240, 240, 240, 240, 240, 240, 240, 240, 207, 207, 207, 207,
204, 204, 204, 193, 193, 180, 143, 66, 40, 185, 185, 185, 185, 185, 185,
185, 185, 193, 193, 193, 193, 193, 193, 193, 183, 183, 172, 138, 65, 39,
207, 207, 207, 207, 207, 207, 207, 207, 204, 204, 204, 204, 201, 201, 201,
188, 188, 176, 141, 66, 40, 193, 193, 193, 193, 193, 193, 193, 193, 193,
193, 193, 193, 194, 194, 194, 184, 184, 173, 139, 65, 39, 204, 204, 204,
204, 204, 204, 204, 204, 201, 201, 201, 201, 198, 198, 198, 187, 187, 175,
140, 66, 40, };
#endif

#ifndef FFT_TWIDDLES48000_960
#define FFT_TWIDDLES48000_960
static const kiss_twiddle_cpx fft_twiddles48000_960[480] = {
{1.0000000f, -0.0000000f}, {0.99991433f, -0.013089596f},
{0.99965732f, -0.026176948f}, {0.99922904f, -0.039259816f},
{0.99862953f, -0.052335956f}, {0.99785892f, -0.065403129f},
{0.99691733f, -0.078459096f}, {0.99580493f, -0.091501619f},
{0.99452190f, -0.10452846f}, {0.99306846f, -0.11753740f},
{0.99144486f, -0.13052619f}, {0.98965139f, -0.14349262f},
{0.98768834f, -0.15643447f}, {0.98555606f, -0.16934950f},
{0.98325491f, -0.18223553f}, {0.98078528f, -0.19509032f},
{0.97814760f, -0.20791169f}, {0.97534232f, -0.22069744f},
{0.97236992f, -0.23344536f}, {0.96923091f, -0.24615329f},
{0.96592583f, -0.25881905f}, {0.96245524f, -0.27144045f},
{0.95881973f, -0.28401534f}, {0.95501994f, -0.29654157f},
{0.95105652f, -0.30901699f}, {0.94693013f, -0.32143947f},
{0.94264149f, -0.33380686f}, {0.93819134f, -0.34611706f},
{0.93358043f, -0.35836795f}, {0.92880955f, -0.37055744f},
{0.92387953f, -0.38268343f}, {0.91879121f, -0.39474386f},
{0.91354546f, -0.40673664f}, {0.90814317f, -0.41865974f},
{0.90258528f, -0.43051110f}, {0.89687274f, -0.44228869f},
{0.89100652f, -0.45399050f}, {0.88498764f, -0.46561452f},
{0.87881711f, -0.47715876f}, {0.87249601f, -0.48862124f},
{0.86602540f, -0.50000000f}, {0.85940641f, -0.51129309f},
{0.85264016f, -0.52249856f}, {0.84572782f, -0.53361452f},
{0.83867057f, -0.54463904f}, {0.83146961f, -0.55557023f},
{0.82412619f, -0.56640624f}, {0.81664156f, -0.57714519f},
{0.80901699f, -0.58778525f}, {0.80125381f, -0.59832460f},
{0.79335334f, -0.60876143f}, {0.78531693f, -0.61909395f},
{0.77714596f, -0.62932039f}, {0.76884183f, -0.63943900f},
{0.76040597f, -0.64944805f}, {0.75183981f, -0.65934582f},
{0.74314483f, -0.66913061f}, {0.73432251f, -0.67880075f},
{0.72537437f, -0.68835458f}, {0.71630194f, -0.69779046f},
{0.70710678f, -0.70710678f}, {0.69779046f, -0.71630194f},
{0.68835458f, -0.72537437f}, {0.67880075f, -0.73432251f},
{0.66913061f, -0.74314483f}, {0.65934582f, -0.75183981f},
{0.64944805f, -0.76040597f}, {0.63943900f, -0.76884183f},
{0.62932039f, -0.77714596f}, {0.61909395f, -0.78531693f},
{0.60876143f, -0.79335334f}, {0.59832460f, -0.80125381f},
{0.58778525f, -0.80901699f}, {0.57714519f, -0.81664156f},
{0.56640624f, -0.82412619f}, {0.55557023f, -0.83146961f},
{0.54463904f, -0.83867057f}, {0.53361452f, -0.84572782f},
{0.52249856f, -0.85264016f}, {0.51129309f, -0.85940641f},
{0.50000000f, -0.86602540f}, {0.48862124f, -0.87249601f},
{0.47715876f, -0.87881711f}, {0.46561452f, -0.88498764f},
{0.45399050f, -0.89100652f}, {0.44228869f, -0.89687274f},
{0.43051110f, -0.90258528f}, {0.41865974f, -0.90814317f},
{0.40673664f, -0.91354546f}, {0.39474386f, -0.91879121f},
{0.38268343f, -0.92387953f}, {0.37055744f, -0.92880955f},
{0.35836795f, -0.93358043f}, {0.34611706f, -0.93819134f},
{0.33380686f, -0.94264149f}, {0.32143947f, -0.94693013f},
{0.30901699f, -0.95105652f}, {0.29654157f, -0.95501994f},
{0.28401534f, -0.95881973f}, {0.27144045f, -0.96245524f},
{0.25881905f, -0.96592583f}, {0.24615329f, -0.96923091f},
{0.23344536f, -0.97236992f}, {0.22069744f, -0.97534232f},
{0.20791169f, -0.97814760f}, {0.19509032f, -0.98078528f},
{0.18223553f, -0.98325491f}, {0.16934950f, -0.98555606f},
{0.15643447f, -0.98768834f}, {0.14349262f, -0.98965139f},
{0.13052619f, -0.99144486f}, {0.11753740f, -0.99306846f},
{0.10452846f, -0.99452190f}, {0.091501619f, -0.99580493f},
{0.078459096f, -0.99691733f}, {0.065403129f, -0.99785892f},
{0.052335956f, -0.99862953f}, {0.039259816f, -0.99922904f},
{0.026176948f, -0.99965732f}, {0.013089596f, -0.99991433f},
{6.1230318e-17f, -1.0000000f}, {-0.013089596f, -0.99991433f},
{-0.026176948f, -0.99965732f}, {-0.039259816f, -0.99922904f},
{-0.052335956f, -0.99862953f}, {-0.065403129f, -0.99785892f},
{-0.078459096f, -0.99691733f}, {-0.091501619f, -0.99580493f},
{-0.10452846f, -0.99452190f}, {-0.11753740f, -0.99306846f},
{-0.13052619f, -0.99144486f}, {-0.14349262f, -0.98965139f},
{-0.15643447f, -0.98768834f}, {-0.16934950f, -0.98555606f},
{-0.18223553f, -0.98325491f}, {-0.19509032f, -0.98078528f},
{-0.20791169f, -0.97814760f}, {-0.22069744f, -0.97534232f},
{-0.23344536f, -0.97236992f}, {-0.24615329f, -0.96923091f},
{-0.25881905f, -0.96592583f}, {-0.27144045f, -0.96245524f},
{-0.28401534f, -0.95881973f}, {-0.29654157f, -0.95501994f},
{-0.30901699f, -0.95105652f}, {-0.32143947f, -0.94693013f},
{-0.33380686f, -0.94264149f}, {-0.34611706f, -0.93819134f},
{-0.35836795f, -0.93358043f}, {-0.37055744f, -0.92880955f},
{-0.38268343f, -0.92387953f}, {-0.39474386f, -0.91879121f},
{-0.40673664f, -0.91354546f}, {-0.41865974f, -0.90814317f},
{-0.43051110f, -0.90258528f}, {-0.44228869f, -0.89687274f},
{-0.45399050f, -0.89100652f}, {-0.46561452f, -0.88498764f},
{-0.47715876f, -0.87881711f}, {-0.48862124f, -0.87249601f},
{-0.50000000f, -0.86602540f}, {-0.51129309f, -0.85940641f},
{-0.52249856f, -0.85264016f}, {-0.53361452f, -0.84572782f},
{-0.54463904f, -0.83867057f}, {-0.55557023f, -0.83146961f},
{-0.56640624f, -0.82412619f}, {-0.57714519f, -0.81664156f},
{-0.58778525f, -0.80901699f}, {-0.59832460f, -0.80125381f},
{-0.60876143f, -0.79335334f}, {-0.61909395f, -0.78531693f},
{-0.62932039f, -0.77714596f}, {-0.63943900f, -0.76884183f},
{-0.64944805f, -0.76040597f}, {-0.65934582f, -0.75183981f},
{-0.66913061f, -0.74314483f}, {-0.67880075f, -0.73432251f},
{-0.68835458f, -0.72537437f}, {-0.69779046f, -0.71630194f},
{-0.70710678f, -0.70710678f}, {-0.71630194f, -0.69779046f},
{-0.72537437f, -0.68835458f}, {-0.73432251f, -0.67880075f},
{-0.74314483f, -0.66913061f}, {-0.75183981f, -0.65934582f},
{-0.76040597f, -0.64944805f}, {-0.76884183f, -0.63943900f},
{-0.77714596f, -0.62932039f}, {-0.78531693f, -0.61909395f},
{-0.79335334f, -0.60876143f}, {-0.80125381f, -0.59832460f},
{-0.80901699f, -0.58778525f}, {-0.81664156f, -0.57714519f},
{-0.82412619f, -0.56640624f}, {-0.83146961f, -0.55557023f},
{-0.83867057f, -0.54463904f}, {-0.84572782f, -0.53361452f},
{-0.85264016f, -0.52249856f}, {-0.85940641f, -0.51129309f},
{-0.86602540f, -0.50000000f}, {-0.87249601f, -0.48862124f},
{-0.87881711f, -0.47715876f}, {-0.88498764f, -0.46561452f},
{-0.89100652f, -0.45399050f}, {-0.89687274f, -0.44228869f},
{-0.90258528f, -0.43051110f}, {-0.90814317f, -0.41865974f},
{-0.91354546f, -0.40673664f}, {-0.91879121f, -0.39474386f},
{-0.92387953f, -0.38268343f}, {-0.92880955f, -0.37055744f},
{-0.93358043f, -0.35836795f}, {-0.93819134f, -0.34611706f},
{-0.94264149f, -0.33380686f}, {-0.94693013f, -0.32143947f},
{-0.95105652f, -0.30901699f}, {-0.95501994f, -0.29654157f},
{-0.95881973f, -0.28401534f}, {-0.96245524f, -0.27144045f},
{-0.96592583f, -0.25881905f}, {-0.96923091f, -0.24615329f},
{-0.97236992f, -0.23344536f}, {-0.97534232f, -0.22069744f},
{-0.97814760f, -0.20791169f}, {-0.98078528f, -0.19509032f},
{-0.98325491f, -0.18223553f}, {-0.98555606f, -0.16934950f},
{-0.98768834f, -0.15643447f}, {-0.98965139f, -0.14349262f},
{-0.99144486f, -0.13052619f}, {-0.99306846f, -0.11753740f},
{-0.99452190f, -0.10452846f}, {-0.99580493f, -0.091501619f},
{-0.99691733f, -0.078459096f}, {-0.99785892f, -0.065403129f},
{-0.99862953f, -0.052335956f}, {-0.99922904f, -0.039259816f},
{-0.99965732f, -0.026176948f}, {-0.99991433f, -0.013089596f},
{-1.0000000f, -1.2246064e-16f}, {-0.99991433f, 0.013089596f},
{-0.99965732f, 0.026176948f}, {-0.99922904f, 0.039259816f},
{-0.99862953f, 0.052335956f}, {-0.99785892f, 0.065403129f},
{-0.99691733f, 0.078459096f}, {-0.99580493f, 0.091501619f},
{-0.99452190f, 0.10452846f}, {-0.99306846f, 0.11753740f},
{-0.99144486f, 0.13052619f}, {-0.98965139f, 0.14349262f},
{-0.98768834f, 0.15643447f}, {-0.98555606f, 0.16934950f},
{-0.98325491f, 0.18223553f}, {-0.98078528f, 0.19509032f},
{-0.97814760f, 0.20791169f}, {-0.97534232f, 0.22069744f},
{-0.97236992f, 0.23344536f}, {-0.96923091f, 0.24615329f},
{-0.96592583f, 0.25881905f}, {-0.96245524f, 0.27144045f},
{-0.95881973f, 0.28401534f}, {-0.95501994f, 0.29654157f},
{-0.95105652f, 0.30901699f}, {-0.94693013f, 0.32143947f},
{-0.94264149f, 0.33380686f}, {-0.93819134f, 0.34611706f},
{-0.93358043f, 0.35836795f}, {-0.92880955f, 0.37055744f},
{-0.92387953f, 0.38268343f}, {-0.91879121f, 0.39474386f},
{-0.91354546f, 0.40673664f}, {-0.90814317f, 0.41865974f},
{-0.90258528f, 0.43051110f}, {-0.89687274f, 0.44228869f},
{-0.89100652f, 0.45399050f}, {-0.88498764f, 0.46561452f},
{-0.87881711f, 0.47715876f}, {-0.87249601f, 0.48862124f},
{-0.86602540f, 0.50000000f}, {-0.85940641f, 0.51129309f},
{-0.85264016f, 0.52249856f}, {-0.84572782f, 0.53361452f},
{-0.83867057f, 0.54463904f}, {-0.83146961f, 0.55557023f},
{-0.82412619f, 0.56640624f}, {-0.81664156f, 0.57714519f},
{-0.80901699f, 0.58778525f}, {-0.80125381f, 0.59832460f},
{-0.79335334f, 0.60876143f}, {-0.78531693f, 0.61909395f},
{-0.77714596f, 0.62932039f}, {-0.76884183f, 0.63943900f},
{-0.76040597f, 0.64944805f}, {-0.75183981f, 0.65934582f},
{-0.74314483f, 0.66913061f}, {-0.73432251f, 0.67880075f},
{-0.72537437f, 0.68835458f}, {-0.71630194f, 0.69779046f},
{-0.70710678f, 0.70710678f}, {-0.69779046f, 0.71630194f},
{-0.68835458f, 0.72537437f}, {-0.67880075f, 0.73432251f},
{-0.66913061f, 0.74314483f}, {-0.65934582f, 0.75183981f},
{-0.64944805f, 0.76040597f}, {-0.63943900f, 0.76884183f},
{-0.62932039f, 0.77714596f}, {-0.61909395f, 0.78531693f},
{-0.60876143f, 0.79335334f}, {-0.59832460f, 0.80125381f},
{-0.58778525f, 0.80901699f}, {-0.57714519f, 0.81664156f},
{-0.56640624f, 0.82412619f}, {-0.55557023f, 0.83146961f},
{-0.54463904f, 0.83867057f}, {-0.53361452f, 0.84572782f},
{-0.52249856f, 0.85264016f}, {-0.51129309f, 0.85940641f},
{-0.50000000f, 0.86602540f}, {-0.48862124f, 0.87249601f},
{-0.47715876f, 0.87881711f}, {-0.46561452f, 0.88498764f},
{-0.45399050f, 0.89100652f}, {-0.44228869f, 0.89687274f},
{-0.43051110f, 0.90258528f}, {-0.41865974f, 0.90814317f},
{-0.40673664f, 0.91354546f}, {-0.39474386f, 0.91879121f},
{-0.38268343f, 0.92387953f}, {-0.37055744f, 0.92880955f},
{-0.35836795f, 0.93358043f}, {-0.34611706f, 0.93819134f},
{-0.33380686f, 0.94264149f}, {-0.32143947f, 0.94693013f},
{-0.30901699f, 0.95105652f}, {-0.29654157f, 0.95501994f},
{-0.28401534f, 0.95881973f}, {-0.27144045f, 0.96245524f},
{-0.25881905f, 0.96592583f}, {-0.24615329f, 0.96923091f},
{-0.23344536f, 0.97236992f}, {-0.22069744f, 0.97534232f},
{-0.20791169f, 0.97814760f}, {-0.19509032f, 0.98078528f},
{-0.18223553f, 0.98325491f}, {-0.16934950f, 0.98555606f},
{-0.15643447f, 0.98768834f}, {-0.14349262f, 0.98965139f},
{-0.13052619f, 0.99144486f}, {-0.11753740f, 0.99306846f},
{-0.10452846f, 0.99452190f}, {-0.091501619f, 0.99580493f},
{-0.078459096f, 0.99691733f}, {-0.065403129f, 0.99785892f},
{-0.052335956f, 0.99862953f}, {-0.039259816f, 0.99922904f},
{-0.026176948f, 0.99965732f}, {-0.013089596f, 0.99991433f},
{-1.8369095e-16f, 1.0000000f}, {0.013089596f, 0.99991433f},
{0.026176948f, 0.99965732f}, {0.039259816f, 0.99922904f},
{0.052335956f, 0.99862953f}, {0.065403129f, 0.99785892f},
{0.078459096f, 0.99691733f}, {0.091501619f, 0.99580493f},
{0.10452846f, 0.99452190f}, {0.11753740f, 0.99306846f},
{0.13052619f, 0.99144486f}, {0.14349262f, 0.98965139f},
{0.15643447f, 0.98768834f}, {0.16934950f, 0.98555606f},
{0.18223553f, 0.98325491f}, {0.19509032f, 0.98078528f},
{0.20791169f, 0.97814760f}, {0.22069744f, 0.97534232f},
{0.23344536f, 0.97236992f}, {0.24615329f, 0.96923091f},
{0.25881905f, 0.96592583f}, {0.27144045f, 0.96245524f},
{0.28401534f, 0.95881973f}, {0.29654157f, 0.95501994f},
{0.30901699f, 0.95105652f}, {0.32143947f, 0.94693013f},
{0.33380686f, 0.94264149f}, {0.34611706f, 0.93819134f},
{0.35836795f, 0.93358043f}, {0.37055744f, 0.92880955f},
{0.38268343f, 0.92387953f}, {0.39474386f, 0.91879121f},
{0.40673664f, 0.91354546f}, {0.41865974f, 0.90814317f},
{0.43051110f, 0.90258528f}, {0.44228869f, 0.89687274f},
{0.45399050f, 0.89100652f}, {0.46561452f, 0.88498764f},
{0.47715876f, 0.87881711f}, {0.48862124f, 0.87249601f},
{0.50000000f, 0.86602540f}, {0.51129309f, 0.85940641f},
{0.52249856f, 0.85264016f}, {0.53361452f, 0.84572782f},
{0.54463904f, 0.83867057f}, {0.55557023f, 0.83146961f},
{0.56640624f, 0.82412619f}, {0.57714519f, 0.81664156f},
{0.58778525f, 0.80901699f}, {0.59832460f, 0.80125381f},
{0.60876143f, 0.79335334f}, {0.61909395f, 0.78531693f},
{0.62932039f, 0.77714596f}, {0.63943900f, 0.76884183f},
{0.64944805f, 0.76040597f}, {0.65934582f, 0.75183981f},
{0.66913061f, 0.74314483f}, {0.67880075f, 0.73432251f},
{0.68835458f, 0.72537437f}, {0.69779046f, 0.71630194f},
{0.70710678f, 0.70710678f}, {0.71630194f, 0.69779046f},
{0.72537437f, 0.68835458f}, {0.73432251f, 0.67880075f},
{0.74314483f, 0.66913061f}, {0.75183981f, 0.65934582f},
{0.76040597f, 0.64944805f}, {0.76884183f, 0.63943900f},
{0.77714596f, 0.62932039f}, {0.78531693f, 0.61909395f},
{0.79335334f, 0.60876143f}, {0.80125381f, 0.59832460f},
{0.80901699f, 0.58778525f}, {0.81664156f, 0.57714519f},
{0.82412619f, 0.56640624f}, {0.83146961f, 0.55557023f},
{0.83867057f, 0.54463904f}, {0.84572782f, 0.53361452f},
{0.85264016f, 0.52249856f}, {0.85940641f, 0.51129309f},
{0.86602540f, 0.50000000f}, {0.87249601f, 0.48862124f},
{0.87881711f, 0.47715876f}, {0.88498764f, 0.46561452f},
{0.89100652f, 0.45399050f}, {0.89687274f, 0.44228869f},
{0.90258528f, 0.43051110f}, {0.90814317f, 0.41865974f},
{0.91354546f, 0.40673664f}, {0.91879121f, 0.39474386f},
{0.92387953f, 0.38268343f}, {0.92880955f, 0.37055744f},
{0.93358043f, 0.35836795f}, {0.93819134f, 0.34611706f},
{0.94264149f, 0.33380686f}, {0.94693013f, 0.32143947f},
{0.95105652f, 0.30901699f}, {0.95501994f, 0.29654157f},
{0.95881973f, 0.28401534f}, {0.96245524f, 0.27144045f},
{0.96592583f, 0.25881905f}, {0.96923091f, 0.24615329f},
{0.97236992f, 0.23344536f}, {0.97534232f, 0.22069744f},
{0.97814760f, 0.20791169f}, {0.98078528f, 0.19509032f},
{0.98325491f, 0.18223553f}, {0.98555606f, 0.16934950f},
{0.98768834f, 0.15643447f}, {0.98965139f, 0.14349262f},
{0.99144486f, 0.13052619f}, {0.99306846f, 0.11753740f},
{0.99452190f, 0.10452846f}, {0.99580493f, 0.091501619f},
{0.99691733f, 0.078459096f}, {0.99785892f, 0.065403129f},
{0.99862953f, 0.052335956f}, {0.99922904f, 0.039259816f},
{0.99965732f, 0.026176948f}, {0.99991433f, 0.013089596f},
};
#ifndef FFT_BITREV480
#define FFT_BITREV480
static const opus_int16 fft_bitrev480[480] = {
0, 120, 240, 360, 30, 150, 270, 390, 60, 180, 300, 420, 90, 210, 330,
450, 15, 135, 255, 375, 45, 165, 285, 405, 75, 195, 315, 435, 105, 225,
345, 465, 5, 125, 245, 365, 35, 155, 275, 395, 65, 185, 305, 425, 95,
215, 335, 455, 20, 140, 260, 380, 50, 170, 290, 410, 80, 200, 320, 440,
110, 230, 350, 470, 10, 130, 250, 370, 40, 160, 280, 400, 70, 190, 310,
430, 100, 220, 340, 460, 25, 145, 265, 385, 55, 175, 295, 415, 85, 205,
325, 445, 115, 235, 355, 475, 1, 121, 241, 361, 31, 151, 271, 391, 61,
181, 301, 421, 91, 211, 331, 451, 16, 136, 256, 376, 46, 166, 286, 406,
76, 196, 316, 436, 106, 226, 346, 466, 6, 126, 246, 366, 36, 156, 276,
396, 66, 186, 306, 426, 96, 216, 336, 456, 21, 141, 261, 381, 51, 171,
291, 411, 81, 201, 321, 441, 111, 231, 351, 471, 11, 131, 251, 371, 41,
161, 281, 401, 71, 191, 311, 431, 101, 221, 341, 461, 26, 146, 266, 386,
56, 176, 296, 416, 86, 206, 326, 446, 116, 236, 356, 476, 2, 122, 242,
362, 32, 152, 272, 392, 62, 182, 302, 422, 92, 212, 332, 452, 17, 137,
257, 377, 47, 167, 287, 407, 77, 197, 317, 437, 107, 227, 347, 467, 7,
127, 247, 367, 37, 157, 277, 397, 67, 187, 307, 427, 97, 217, 337, 457,
22, 142, 262, 382, 52, 172, 292, 412, 82, 202, 322, 442, 112, 232, 352,
472, 12, 132, 252, 372, 42, 162, 282, 402, 72, 192, 312, 432, 102, 222,
342, 462, 27, 147, 267, 387, 57, 177, 297, 417, 87, 207, 327, 447, 117,
237, 357, 477, 3, 123, 243, 363, 33, 153, 273, 393, 63, 183, 303, 423,
93, 213, 333, 453, 18, 138, 258, 378, 48, 168, 288, 408, 78, 198, 318,
438, 108, 228, 348, 468, 8, 128, 248, 368, 38, 158, 278, 398, 68, 188,
308, 428, 98, 218, 338, 458, 23, 143, 263, 383, 53, 173, 293, 413, 83,
203, 323, 443, 113, 233, 353, 473, 13, 133, 253, 373, 43, 163, 283, 403,
73, 193, 313, 433, 103, 223, 343, 463, 28, 148, 268, 388, 58, 178, 298,
418, 88, 208, 328, 448, 118, 238, 358, 478, 4, 124, 244, 364, 34, 154,
274, 394, 64, 184, 304, 424, 94, 214, 334, 454, 19, 139, 259, 379, 49,
169, 289, 409, 79, 199, 319, 439, 109, 229, 349, 469, 9, 129, 249, 369,
39, 159, 279, 399, 69, 189, 309, 429, 99, 219, 339, 459, 24, 144, 264,
384, 54, 174, 294, 414, 84, 204, 324, 444, 114, 234, 354, 474, 14, 134,
254, 374, 44, 164, 284, 404, 74, 194, 314, 434, 104, 224, 344, 464, 29,
149, 269, 389, 59, 179, 299, 419, 89, 209, 329, 449, 119, 239, 359, 479,
};
#endif

#ifndef FFT_BITREV240
#define FFT_BITREV240
static const opus_int16 fft_bitrev240[240] = {
0, 60, 120, 180, 15, 75, 135, 195, 30, 90, 150, 210, 45, 105, 165,
225, 5, 65, 125, 185, 20, 80, 140, 200, 35, 95, 155, 215, 50, 110,
170, 230, 10, 70, 130, 190, 25, 85, 145, 205, 40, 100, 160, 220, 55,
115, 175, 235, 1, 61, 121, 181, 16, 76, 136, 196, 31, 91, 151, 211,
46, 106, 166, 226, 6, 66, 126, 186, 21, 81, 141, 201, 36, 96, 156,
216, 51, 111, 171, 231, 11, 71, 131, 191, 26, 86, 146, 206, 41, 101,
161, 221, 56, 116, 176, 236, 2, 62, 122, 182, 17, 77, 137, 197, 32,
92, 152, 212, 47, 107, 167, 227, 7, 67, 127, 187, 22, 82, 142, 202,
37, 97, 157, 217, 52, 112, 172, 232, 12, 72, 132, 192, 27, 87, 147,
207, 42, 102, 162, 222, 57, 117, 177, 237, 3, 63, 123, 183, 18, 78,
138, 198, 33, 93, 153, 213, 48, 108, 168, 228, 8, 68, 128, 188, 23,
83, 143, 203, 38, 98, 158, 218, 53, 113, 173, 233, 13, 73, 133, 193,
28, 88, 148, 208, 43, 103, 163, 223, 58, 118, 178, 238, 4, 64, 124,
184, 19, 79, 139, 199, 34, 94, 154, 214, 49, 109, 169, 229, 9, 69,
129, 189, 24, 84, 144, 204, 39, 99, 159, 219, 54, 114, 174, 234, 14,
74, 134, 194, 29, 89, 149, 209, 44, 104, 164, 224, 59, 119, 179, 239,
};
#endif

#ifndef FFT_BITREV120
#define FFT_BITREV120
static const opus_int16 fft_bitrev120[120] = {
0, 30, 60, 90, 15, 45, 75, 105, 5, 35, 65, 95, 20, 50, 80,
110, 10, 40, 70, 100, 25, 55, 85, 115, 1, 31, 61, 91, 16, 46,
76, 106, 6, 36, 66, 96, 21, 51, 81, 111, 11, 41, 71, 101, 26,
56, 86, 116, 2, 32, 62, 92, 17, 47, 77, 107, 7, 37, 67, 97,
22, 52, 82, 112, 12, 42, 72, 102, 27, 57, 87, 117, 3, 33, 63,
93, 18, 48, 78, 108, 8, 38, 68, 98, 23, 53, 83, 113, 13, 43,
73, 103, 28, 58, 88, 118, 4, 34, 64, 94, 19, 49, 79, 109, 9,
39, 69, 99, 24, 54, 84, 114, 14, 44, 74, 104, 29, 59, 89, 119,
};
#endif

#ifndef FFT_BITREV60
#define FFT_BITREV60
static const opus_int16 fft_bitrev60[60] = {
0, 15, 30, 45, 5, 20, 35, 50, 10, 25, 40, 55, 1, 16, 31,
46, 6, 21, 36, 51, 11, 26, 41, 56, 2, 17, 32, 47, 7, 22,
37, 52, 12, 27, 42, 57, 3, 18, 33, 48, 8, 23, 38, 53, 13,
28, 43, 58, 4, 19, 34, 49, 9, 24, 39, 54, 14, 29, 44, 59,
};
#endif

#ifndef FFT_STATE48000_960_0
#define FFT_STATE48000_960_0
static const kiss_fft_state fft_state48000_960_0 = {
480,    /* nfft */
0.002083333f,   /* scale */
-1,     /* shift */
{4, 120, 4, 30, 2, 15, 3, 5, 5, 1, 0, 0, 0, 0, 0, 0, }, /* factors */
fft_bitrev480,  /* bitrev */
fft_twiddles48000_960,  /* bitrev */
};
#endif

#ifndef FFT_STATE48000_960_1
#define FFT_STATE48000_960_1
static const kiss_fft_state fft_state48000_960_1 = {
240,    /* nfft */
0.004166667f,   /* scale */
1,      /* shift */
{4, 60, 4, 15, 3, 5, 5, 1, 0, 0, 0, 0, 0, 0, 0, 0, },   /* factors */
fft_bitrev240,  /* bitrev */
fft_twiddles48000_960,  /* bitrev */
};
#endif

#ifndef FFT_STATE48000_960_2
#define FFT_STATE48000_960_2
static const kiss_fft_state fft_state48000_960_2 = {
120,    /* nfft */
0.008333333f,   /* scale */
2,      /* shift */
{4, 30, 2, 15, 3, 5, 5, 1, 0, 0, 0, 0, 0, 0, 0, 0, },   /* factors */
fft_bitrev120,  /* bitrev */
fft_twiddles48000_960,  /* bitrev */
};
#endif

#ifndef FFT_STATE48000_960_3
#define FFT_STATE48000_960_3
static const kiss_fft_state fft_state48000_960_3 = {
60,     /* nfft */
0.016666667f,   /* scale */
3,      /* shift */
{4, 15, 3, 5, 5, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },    /* factors */
fft_bitrev60,   /* bitrev */
fft_twiddles48000_960,  /* bitrev */
};
#endif

#endif

#ifndef MDCT_TWIDDLES960
#define MDCT_TWIDDLES960
static const opus_val16 mdct_twiddles960[481] = {
1.0000000f, 0.99999465f, 0.99997858f, 0.99995181f, 0.99991433f,
0.99986614f, 0.99980724f, 0.99973764f, 0.99965732f, 0.99956631f,
0.99946459f, 0.99935216f, 0.99922904f, 0.99909521f, 0.99895068f,
0.99879546f, 0.99862953f, 0.99845292f, 0.99826561f, 0.99806761f,
0.99785892f, 0.99763955f, 0.99740949f, 0.99716875f, 0.99691733f,
0.99665524f, 0.99638247f, 0.99609903f, 0.99580493f, 0.99550016f,
0.99518473f, 0.99485864f, 0.99452190f, 0.99417450f, 0.99381646f,
0.99344778f, 0.99306846f, 0.99267850f, 0.99227791f, 0.99186670f,
0.99144486f, 0.99101241f, 0.99056934f, 0.99011566f, 0.98965139f,
0.98917651f, 0.98869104f, 0.98819498f, 0.98768834f, 0.98717112f,
0.98664333f, 0.98610497f, 0.98555606f, 0.98499659f, 0.98442657f,
0.98384600f, 0.98325491f, 0.98265328f, 0.98204113f, 0.98141846f,
0.98078528f, 0.98014159f, 0.97948742f, 0.97882275f, 0.97814760f,
0.97746197f, 0.97676588f, 0.97605933f, 0.97534232f, 0.97461487f,
0.97387698f, 0.97312866f, 0.97236992f, 0.97160077f, 0.97082121f,
0.97003125f, 0.96923091f, 0.96842019f, 0.96759909f, 0.96676764f,
0.96592582f, 0.96507367f, 0.96421118f, 0.96333837f, 0.96245523f,
0.96156180f, 0.96065806f, 0.95974403f, 0.95881973f, 0.95788517f,
0.95694034f, 0.95598526f, 0.95501995f, 0.95404440f, 0.95305864f,
0.95206267f, 0.95105651f, 0.95004016f, 0.94901364f, 0.94797697f,
0.94693013f, 0.94587315f, 0.94480604f, 0.94372882f, 0.94264149f,
0.94154406f, 0.94043656f, 0.93931897f, 0.93819133f, 0.93705365f,
0.93590592f, 0.93474818f, 0.93358042f, 0.93240268f, 0.93121493f,
0.93001722f, 0.92880955f, 0.92759193f, 0.92636438f, 0.92512690f,
0.92387953f, 0.92262225f, 0.92135509f, 0.92007809f, 0.91879121f,
0.91749449f, 0.91618795f, 0.91487161f, 0.91354545f, 0.91220952f,
0.91086382f, 0.90950836f, 0.90814316f, 0.90676824f, 0.90538363f,
0.90398929f, 0.90258528f, 0.90117161f, 0.89974828f, 0.89831532f,
0.89687273f, 0.89542055f, 0.89395877f, 0.89248742f, 0.89100652f,
0.88951606f, 0.88801610f, 0.88650661f, 0.88498764f, 0.88345918f,
0.88192125f, 0.88037390f, 0.87881711f, 0.87725090f, 0.87567531f,
0.87409035f, 0.87249599f, 0.87089232f, 0.86927933f, 0.86765699f,
0.86602540f, 0.86438453f, 0.86273437f, 0.86107503f, 0.85940641f,
0.85772862f, 0.85604161f, 0.85434547f, 0.85264014f, 0.85092572f,
0.84920218f, 0.84746955f, 0.84572781f, 0.84397704f, 0.84221721f,
0.84044838f, 0.83867056f, 0.83688375f, 0.83508799f, 0.83328325f,
0.83146961f, 0.82964704f, 0.82781562f, 0.82597530f, 0.82412620f,
0.82226820f, 0.82040144f, 0.81852589f, 0.81664154f, 0.81474847f,
0.81284665f, 0.81093620f, 0.80901698f, 0.80708914f, 0.80515262f,
0.80320752f, 0.80125378f, 0.79929149f, 0.79732067f, 0.79534125f,
0.79335335f, 0.79135691f, 0.78935204f, 0.78733867f, 0.78531691f,
0.78328674f, 0.78124818f, 0.77920122f, 0.77714595f, 0.77508232f,
0.77301043f, 0.77093026f, 0.76884183f, 0.76674517f, 0.76464026f,
0.76252720f, 0.76040593f, 0.75827656f, 0.75613907f, 0.75399349f,
0.75183978f, 0.74967807f, 0.74750833f, 0.74533054f, 0.74314481f,
0.74095112f, 0.73874950f, 0.73653993f, 0.73432251f, 0.73209718f,
0.72986405f, 0.72762307f, 0.72537438f, 0.72311787f, 0.72085359f,
0.71858162f, 0.71630192f, 0.71401459f, 0.71171956f, 0.70941701f,
0.70710677f, 0.70478900f, 0.70246363f, 0.70013079f, 0.69779041f,
0.69544260f, 0.69308738f, 0.69072466f, 0.68835458f, 0.68597709f,
0.68359229f, 0.68120013f, 0.67880072f, 0.67639404f, 0.67398011f,
0.67155892f, 0.66913059f, 0.66669509f, 0.66425240f, 0.66180265f,
0.65934581f, 0.65688191f, 0.65441092f, 0.65193298f, 0.64944801f,
0.64695613f, 0.64445727f, 0.64195160f, 0.63943902f, 0.63691954f,
0.63439328f, 0.63186019f, 0.62932037f, 0.62677377f, 0.62422055f,
0.62166055f, 0.61909394f, 0.61652065f, 0.61394081f, 0.61135435f,
0.60876139f, 0.60616195f, 0.60355593f, 0.60094349f, 0.59832457f,
0.59569929f, 0.59306758f, 0.59042957f, 0.58778523f, 0.58513460f,
0.58247766f, 0.57981452f, 0.57714518f, 0.57446961f, 0.57178793f,
0.56910013f, 0.56640624f, 0.56370623f, 0.56100023f, 0.55828818f,
0.55557020f, 0.55284627f, 0.55011641f, 0.54738067f, 0.54463901f,
0.54189157f, 0.53913828f, 0.53637921f, 0.53361450f, 0.53084398f,
0.52806787f, 0.52528601f, 0.52249852f, 0.51970543f, 0.51690688f,
0.51410279f, 0.51129310f, 0.50847793f, 0.50565732f, 0.50283139f,
0.49999997f, 0.49716321f, 0.49432122f, 0.49147383f, 0.48862118f,
0.48576340f, 0.48290042f, 0.48003216f, 0.47715876f, 0.47428025f,
0.47139677f, 0.46850813f, 0.46561448f, 0.46271584f, 0.45981235f,
0.45690383f, 0.45399042f, 0.45107214f, 0.44814915f, 0.44522124f,
0.44228868f, 0.43935137f, 0.43640926f, 0.43346247f, 0.43051104f,
0.42755511f, 0.42459449f, 0.42162932f, 0.41865964f, 0.41568558f,
0.41270697f, 0.40972393f, 0.40673661f, 0.40374494f, 0.40074884f,
0.39774844f, 0.39474390f, 0.39173501f, 0.38872193f, 0.38570469f,
0.38268343f, 0.37965796f, 0.37662842f, 0.37359496f, 0.37055739f,
0.36751585f, 0.36447038f, 0.36142122f, 0.35836797f, 0.35531089f,
0.35225000f, 0.34918544f, 0.34611704f, 0.34304493f, 0.33996926f,
0.33688983f, 0.33380680f, 0.33072019f, 0.32763015f, 0.32453650f,
0.32143936f, 0.31833890f, 0.31523503f, 0.31212767f, 0.30901696f,
0.30590306f, 0.30278577f, 0.29966524f, 0.29654150f, 0.29341470f,
0.29028464f, 0.28715147f, 0.28401522f, 0.28087605f, 0.27773376f,
0.27458861f, 0.27144052f, 0.26828940f, 0.26513541f, 0.26197859f,
0.25881907f, 0.25565666f, 0.25249152f, 0.24932367f, 0.24615327f,
0.24298012f, 0.23980436f, 0.23662604f, 0.23344530f, 0.23026206f,
0.22707623f, 0.22388809f, 0.22069744f, 0.21750443f, 0.21430908f,
0.21111156f, 0.20791165f, 0.20470953f, 0.20150520f, 0.19829884f,
0.19509024f, 0.19187955f, 0.18866692f, 0.18545227f, 0.18223552f,
0.17901681f, 0.17579631f, 0.17257380f, 0.16934945f, 0.16612328f,
0.16289546f, 0.15966577f, 0.15643437f, 0.15320141f, 0.14996669f,
0.14673037f, 0.14349260f, 0.14025329f, 0.13701235f, 0.13376995f,
0.13052612f, 0.12728101f, 0.12403442f, 0.12078650f, 0.11753740f,
0.11428693f, 0.11103523f, 0.10778234f, 0.10452842f, 0.10127326f,
0.098017137f, 0.094759842f, 0.091501652f, 0.088242363f, 0.084982129f,
0.081721103f, 0.078459084f, 0.075196224f, 0.071932560f, 0.068668243f,
0.065403073f, 0.062137201f, 0.058870665f, 0.055603617f, 0.052335974f,
0.049067651f, 0.045798921f, 0.042529582f, 0.039259788f, 0.035989573f,
0.032719092f, 0.029448142f, 0.026176876f, 0.022905329f, 0.019633657f,
0.016361655f, 0.013089478f, 0.0098171604f, 0.0065449764f, 0.0032724839f,
-4.3711390e-08f, };
#endif

static const CELTMode mode48000_960_120 = {
NULL,
48000,  /* Fs */
120,    /* overlap */
21,     /* nbEBands */
21,     /* effEBands */
{0.85000610f, 0.0000000f, 1.0000000f, 1.0000000f, },    /* preemph */
eband5ms,       /* eBands */
3,      /* maxLM */
8,      /* nbShortMdcts */
120,    /* shortMdctSize */
11,     /* nbAllocVectors */
band_allocation,        /* allocVectors */
logN400,        /* logN */
window120,      /* window */
{1920, 3, {&fft_state48000_960_0, &fft_state48000_960_1, &fft_state48000_960_2, &fft_state48000_960_3, }, mdct_twiddles960},    /* mdct */
{392, cache_index50, cache_bits50, cache_caps50},       /* cache */
};

/* List of all the available modes */
#define TOTAL_MODES 1
static const CELTMode * const static_mode_list[TOTAL_MODES] = {
&mode48000_960_120,
};
