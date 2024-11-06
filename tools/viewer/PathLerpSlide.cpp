/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cmath>
#include <utility>
#include <deque>
#include <vector>

#include "include/core/SkCanvas.h"
#include "include/core/SkCubicMap.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathMeasure.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/private/SkPathRef.h"
#include "include/private/base/SkAssert.h"
#include "include/utils/SkParsePath.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkPathPriv.h"
#include "tools/viewer/Slide.h"

#include "imgui.h"

namespace {

static constexpr struct PathDesc {
    const char* fName;
    const char* fSVGString;
} gSamplePaths[] = {
    { "Arc 1", "M0,0.5 Q0,0 0.5,0 Q1,0 1,0.5"},
    { "Arc 2", "M0,0.5 Q0,1 0.5,1 Q1,1 1,0.5 L1.1,0.5 L1.2,0.5 L1.3,0.5 L1.5,0.5"},
    { "Pentagon",
        R"(M10.1192 4.09438C10.7952 3.60324 11.1332 3.35767 11.5027 3.26278C11.829 3.17901 12.1712 3.17901 12.4975
        3.26278C12.867 3.35767 13.205 3.60324 13.881 4.09438L19.6298 8.27108C20.3058 8.76222 20.6437 9.0078 20.8482
        9.32992C21.0287 9.61436 21.1344 9.93978 21.1556 10.276C21.1795 10.6568 21.0504 11.0541 20.7922 11.8488L18.5964
        18.6068C18.3382 19.4015 18.2091 19.7989 17.9659 20.0928C17.7512 20.3524 17.4743 20.5535 17.1611 20.6775C16.8064
        20.818 16.3886 20.818 15.553 20.818H8.44718C7.6116 20.818 7.19381 20.818 6.83908 20.6775C6.52586 20.5535 6.24904
        20.3524 6.0343 20.0928C5.79111 19.7989 5.66201 19.4015 5.4038 18.6068L3.20798 11.8488C2.94977 11.0541 2.82066
        10.6568 2.84462 10.276C2.86577 9.93978 2.97151 9.61436 3.15202 9.32992C3.35645 9.0078 3.69445 8.76222 4.37045
        8.27108L10.1192 4.09438Z)"},
    { "Droplet",
        R"(M12 21.5C13.8565 21.5 15.637 20.7625 16.9497 19.4497C18.2625 18.137 19 16.3565 19 14.5C19 12.5 18 10.6 16 9
        C14 7.4 12.5 5 12 2.5C11.5 5 10 7.4 8 9C6 10.6 5 12.5 5 14.5C5 16.3565 5.7375 18.137 7.05025 19.4497
        C8.36301 20.7625 10.1435 21.5 12 21.5V21.5Z)"},
    { "Shield",
        R"(M20 6C20 6 19.1843 6 19.0001 6C16.2681 6 13.8871 4.93485 11.9999 3C10.1128 4.93478 7.73199 6 5.00009 6
        C4.81589 6 4.00009 6 4.00009 6C4.00009 6 4 8 4 9.16611C4 14.8596 7.3994 19.6436 12 21C16.6006 19.6436 20 14.8596 20 9.16611
        C20 8 20 6 20 6Z)"},
    { "Spiral",
        R"(M297,7888.302 L297,7886.302 C297,7885.75 296.552,7885 296,7885 L292,7885 C291.448,7885 291,7885.75 291,7886.302
        L291,7892.302 C291,7892.854 291.448,7893 292,7893 L300,7893 C300.552,7893 301,7892.854 301,7892.302 L301,7882.302
        C301,7881.75 300.552,7881 300,7881 L288,7881 C287.448,7881 287,7881.75 287,7882.302 L287,7896.302
        C287,7896.854 287.448,7897 288,7897 L302,7897 C302.552,7897 303,7897.524 303,7898.076 L303,7898.151
        C303,7898.703 302.552,7899 302,7899 L287,7899 C285.896,7899 285,7898.407 285,7897.302 L285,7881.302
        C285,7880.197 285.896,7879 287,7879 L301,7879 C302.105,7879 303,7880.197 303,7881.302 L303,7893.302
        C303,7894.407 302.105,7895 301,7895 L291,7895 C289.896,7895 289,7894.407 289,7893.302 L289,7885.302
        C289,7884.197 289.896,7883 291,7883 L297,7883 C298.105,7883 299,7884.197 299,7885.302 L299,7889.302
        C299,7890.407 298.105,7891 297,7891 L295,7891 C293.896,7891 293,7890.407 293,7889.302 L293,7888.302
        C293,7887.75 293.448,7887.302 294,7887.302 C294.552,7887.302 295,7887.75 295,7888.302
        C295,7888.854 295.448,7889.302 296,7889.302 C296.552,7889.302 297,7888.854 297,7888.302)"},
    { "Cat",
        R"(M26.753,210.905c-4.68,19.271-1.375,33.856,8.258,46.243c9.633,12.387-1.102,19.543-9.082,23.398
		s-11.012,6.332-11.562,11.836s6.883,26.978,10.461,30.556s11.563,11.285,16.789,19.27c5.231,7.98,12.11,2.754,12.11,2.754
		c4.953,3.855,9.91,0,9.91,0c8.809,4.129,14.039-1.652,14.039-1.652c4.402-5.23-4.953-10.734-4.953-10.734
		c-5.781-10.184-10.184-1.926-11.836-4.129s-3.027-2.477-11.836-12.66s14.039-6.883,22.02-8.809s23.122-12.109,23.122-12.109
		c0,12.109,4.129,10.184,10.184,16.516s8.809,10.461,16.789,13.762c7.98,3.305,11.836,2.754,18.993,7.434s22.02,3.855,22.02,3.855
		c6.605,2.204,12.938,0,12.938,0c15.965,0,9.359-5.504,3.578-12.938s-17.34-1.102-22.57-4.68s-18.168-9.082-24.774-12.938
		c-6.605-3.855,0-10.734,0-17.066s4.953-11.285,4.953-11.285h25.598c0,0,19.816-1.375,25.047,9.359s13.211,12.938,13.488,18.719
		c0.278,5.781-4.68,6.332-4.68,6.332c-11.012-3.027-11.285,14.59-11.285,14.59c13.488,17.892,39.638-7.434,39.638-7.434
		c30.277-2.754,28.074-47.344,28.074-47.344c6.055-4.402,15.691-15.965,22.02-26.427c6.332-10.461,24.223-18.441,40.188-10.734
		c15.965,7.707,18.719-5.23,18.719-5.23c5.504,0.824,7.156-6.332,7.156-6.332c7.156-7.98-5.504-9.91-5.781-29.727
		s-16.789-23.399-15.965-29.176c0.824-5.781,17.617-24.223,13.488-22.571c-3.603,1.44-18.739,8.964-22.53,10.853
		c-0.126-0.498-0.375-1.208-0.868-2.317c-2.204-4.953-26.427,10.734-26.427,10.734s-38.396,22.709-45.83,24.362
		c-7.435,1.652-6.605-3.305-15.688-2.477c-9.083,0.824-13.628,1.652-54.501-15.692c-40.877-17.34-115.325,10.546-115.325,10.546
		S-8.608,82.798,40.935,41.924S24.215-33.009,8.111,56.167C-7.992,145.344,31.433,191.64,26.753,210.905z)" },
    { "Rabit",
        R"(M526.834,235.822c-6.308-15.153-11.856-30.576-18.748-45.546c-5.969-12.966-15.985-20.689-27.282-27.287
		c-6.455-3.766-13.681-6.205-20.563-8.865c-2.102-0.812-3.447-3.309-3.007-5.517c1.126-5.643,2.379-11.607,3.187-17.629
		c0.489-3.664-0.425-7.495-0.47-11.253c-0.061-5.101-0.012-10.204,0.196-15.3c0.363-8.829-1.918-17.157-4.039-25.614
		c-1.359-5.423-0.367-11.478-1.959-16.789c-1.652-5.521,0.322-12.212-4.961-16.614c-0.118-0.098-0.184-0.343-0.167-0.51
		c0.844-8.576-5.484-14.574-8.373-21.714c-2.933-7.246-7.752-14.157-15.879-17.617c-2.881-1.225-5.516-3.023-8.482-4.974
		c-1.881-1.24-4.007-0.466-4.745,1.664c-2.546,7.364-5.116,14.035-6.622,20.935c-1.488,6.847-5.111,13.048-3.998,20.898
		c0.8,5.618-2.685,11.771-3.88,17.764c-0.633,3.17-0.963,6.397-1.302,9.649c-0.232,2.24-1.656,2.685-3.06,0.918
		c-3.366-4.243-6.328-8.507-10.38-11.004c-6.977-4.296-12.162-10.689-18.996-14.745c-10.674-6.332-21.519-12.917-33.138-16.956
		c-11.519-4.007-24.027-5.463-36.23-7.047c-3.212-0.416-6.643,0.845-10.507,2.049c-2.15,0.669-3.076,2.713-2.036,4.712
		c3.594,6.908,6.817,13.358,10.457,19.563c6.027,10.278,12.286,20.433,18.813,30.4c3.701,5.651,8.192,10.779,12.232,16.214
		c4.153,5.594,7.988,11.437,12.342,16.867c3.015,3.758,6.744,6.936,10.017,10.497c3.606,3.926,7.299,7.802,10.526,12.028
		c8.841,11.579,12.276,25.182,14.116,39.319c0.069,0.538,0.151,1.117,0.396,1.591c4.636,8.895-0.236,14.582-6.527,19.935
		c-0.886,0.755-0.959,2.444-1.453,3.685c-0.844,2.113-1.313,4.533-2.66,6.259c-4.174,5.332-4.953,11.493-5.471,17.911
		c-0.322,3.978-1.502,7.874-2.068,11.844c-0.09,0.637-0.041,1.294,0.053,1.901c0.159,1.041-0.534,3.477-1.469,5.528
		c-3.473,7.634-5.899,16.026-13.064,21.172c-1.832,1.313-2.35,1.921-0.91,1.75c1.44-0.171,2.641-0.18,2.686-0.021
		s-1.637,0.926-3.746,1.714c-0.318,0.118-0.641,0.236-0.963,0.358c-2.113,0.788-2.848,1.689-1.644,2.016
		c1.203,0.327,0.461,1.2-1.665,1.955c-1.889,0.669-3.709,1.313-5.569,1.975c-2.126,0.75-3.035,1.767-2.036,2.264
		c1,0.498,0.245,1.294-1.681,1.779s-3.272,1.375-3.003,1.979c0.27,0.604-1.306,1.444-3.509,1.91
		c-3.966,0.836-7.939,1.701-11.963,2.068c-1.301,0.118-2.7-0.086-4.08-0.478c-2.166-0.616-5.605-1.624-7.858-1.575
		c-13.17,0.282-35.672,4.391-44.778,6.148c-2.211,0.429-5.818,0.979-8.062,1.212c-3.052,0.314-6.308,0.649-9.666,0.996
		c-2.24,0.232-3.77,0.689-3.411,1.023c0.355,0.335-1.089,1.184-3.228,1.897c-9.833,3.288-19.849,6.634-29.906,9.996
		c-2.138,0.714-3.297,1.611-2.583,2.003c0.714,0.392-0.4,1.403-2.521,2.171c-5.675,2.048-11.355,3.725-15.802,7.054
		c-4.814,3.607-10.164,6.479-14.913,10.286c-5.349,4.284-12.795,5.944-19.331,8.747c-0.62,0.266-1.24,0.53-2.056,0.886
		c-1.139,0.489-1.155,1.293-0.033,1.795s0.563,1.999-1.245,3.346c-3.929,2.922-7.813,5.814-11.738,8.735
		c-1.808,1.347-2.734,2.746-2.069,3.129c0.665,0.384-0.396,1.571-2.371,2.656c-1.33,0.73-2.64,1.448-3.941,2.167
		c-1.975,1.085-3.484,2.146-3.375,2.37c0.114,0.225,1.424,0.188,2.934-0.086c1.505-0.273,1.408,0.771-0.22,2.326
		c-7.181,6.854-14.125,13.492-21.094,20.106c-0.718,0.681-1.481,1.313-2.436,2.101c-1.375,1.131-1.008,1.648,0.808,1.146
		c1.816-0.497,2.272,0.62,1.044,2.51c-3.472,5.32-6.842,10.579-10.453,15.667c-2.317,3.26-5.271,6.071-7.899,9.114
		c-0.082,0.099-0.065,0.282-0.029,0.485c0.045,0.273,0.796,0.359,1.681,0.196s0.526,1.179-0.8,3.003
		c-2.199,3.015-4.345,5.965-6.679,9.168c-1.326,1.819-0.947,2.518,0.844,1.555c1.792-0.963,2.518-0.062,1.559,1.979
		c-3.537,7.52-7.226,14.052-7.789,21.93c-0.277,3.88-2.317,7.63-4.304,11.824c-0.963,2.036-0.853,3.162,0.126,2.46
		c0.979-0.702,1.469,0.534,1.085,2.754c-1.812,10.457-3.578,20.669-5.328,30.768c-0.384,2.22-0.106,3.773,0.624,3.468
		s0.812,1.199,0.184,3.366c-0.498,1.709-0.979,3.361-1.457,5.015c-0.628,2.162-0.481,3.745,0.322,3.533
		c0.808-0.213,0.795,1.334,0.404,3.554c-1.02,5.797-0.355,11.938-4.602,16.703c-1.498,1.681-2.236,2.628-1.159,2.628
		s0.877,1.48-0.441,3.312c-0.359,0.498-0.714,0.995-1.081,1.501c-1.318,1.828-1.665,3.068-0.779,2.763s1.677-0.44,1.767-0.306
		c0.062,0.089,0.11,0.175,0.114,0.261c0.029,0.538,0.057,1.085-0.004,1.615c-1.134,9.771,2.023,17.349,11.33,21.555
		c6.749,3.052,11.62,9.915,20.433,9.078c2.762-0.261,5.822,2.709,8.764,4.158c0.114,0.057,0.286,0.004,0.469-0.07
		c0.253-0.098,0.265-0.628,0.029-1.187c-0.237-0.56,1.358-0.641,3.566-0.176c1.583,0.335,3.17,0.669,4.773,1.008
		c2.208,0.465,3.705,0.229,3.354-0.526c-0.351-0.751,1.061-0.673,3.199,0.033c18.458,6.091,37.271,3.562,55.928,3.912
		c2.252,0.041,3.832-0.229,3.534-0.661c-0.302-0.437,1.285-0.787,3.542-0.783c6.417,0.013,12.889,0.037,19.355-0.032
		c3.007-0.033,6.03-0.217,9.008-0.62c6.626-0.894,13.244-1.849,19.829-2.991c7.222-1.252,14.517,1.604,21.962-2.746
		c4.357-2.545,8.319-4.476,9.854-7.833c0.934-2.048,2.415-4.423,4.61-4.941c4.08-0.967,7.936-1.848,11.706-3.007
		c5.283-1.623,9.849-3.541,10.967-10.521c0.792-4.958,0.396-9.507-4.1-12.146c-0.441-0.257-0.682-0.849-0.849-1.469
		c-0.245-0.927-2.211-1.849-4.456-2.036c-0.498-0.041-1.008-0.086-1.53-0.131c-2.244-0.188-3.125-1.905-1.962-3.839
		c0.673-1.118,1.302-2.162,1.905-3.17c1.163-1.93,2.375-3.35,2.709-3.175c0.334,0.176,0.702,1.657,0.816,3.31
		s1.02,1.362,2.016-0.661c0.489-0.991,0.958-1.942,1.412-2.868c0.996-2.02,2.048-3.574,2.346-3.469
		c0.298,0.106,0.673,1.506,0.833,3.117c0.159,1.612,0.967,1.225,1.808-0.869c0.269-0.673,0.539-1.342,0.812-2.027
		c0.836-2.093,1.636-2.276,1.787-0.416c0.151,1.86,0.71,1.583,1.64-0.474c1.856-4.101,4.762-7.188,9.323-9.127
		c6.304-2.676,12.371-5.916,18.675-8.604c1.061-0.453,3.337,0.2,4.104,1.126c3.08,3.705,5.699,7.789,8.666,11.6
		c3.112,4.003,6.344,7.919,9.673,11.742c4.239,4.863,8.63,9.592,12.946,14.39c0.245,0.273,0.461,0.567,0.686,0.882
		c0.354,0.506,0.967,0.399,1.362-0.229s1.457,0.53,2.375,2.591c0.322,0.722,0.648,1.456,0.983,2.216
		c0.913,2.06,1.827,2.599,2.035,1.203s1.412-1.024,2.685,0.832c3.396,4.953,6.684,9.752,9.891,14.427
		c1.272,1.861,2.305,2.203,2.305,0.771s1.024-1.077,2.285,0.787c1.885,2.787,3.676,5.431,5.59,8.258
		c1.265,1.865,2.093,1.946,1.86,0.18c-0.236-1.767-0.139-3.406,0.212-3.659c0.351-0.257,1.714,1.012,3.04,2.831
		c0.755,1.032,1.505,2.064,2.256,3.097c1.326,1.82,2.844,2.844,3.293,2.216c0.465-0.648,0.51-0.959,0.53-1.27
		c0.012-0.171-0.139-0.354-0.123-0.525c0.005-0.065,0.09-0.119,0.241-0.221c0.15-0.102,1.75,0.877,3.578,2.195
		c1.248,0.897,2.514,1.815,3.818,2.754c1.828,1.317,3.224,1.562,3.117,0.551c-0.105-1.012,1.498-1.135,3.562-0.232
		c1.782,0.779,3.479,1.469,5.242,1.746c4.517,0.718,9.393-0.147,13.705,1.089c8.629,2.469,15.851-1.326,21.596-5.899
		c3.603-2.868,6.98-3.644,10.901-3.998c2.244-0.204,5.397-1.53,7.034-3.081c2.244-2.13,4.545-4.316,6.72-6.377
		c1.632-1.55,2.179-4.459,1.167-6.475c-2.525-5.035-5.292-9.417-10.698-11.021c-2.158-0.641-3.178-1.338-2.142-2.207
		s0.049-1.575-2.203-1.575c-0.629,0-1.236,0-1.828,0c-2.252,0-4.08-0.114-4.084-0.257s0.881-0.494,1.982-0.787
		c1.102-0.294,0.339-1.302-1.705-2.252c-0.612-0.282-1.212-0.563-1.812-0.841c-2.044-0.946-2.905-1.909-1.922-2.142
		c0.983-0.237,0.172-1.298-1.812-2.371c-3.741-2.031-7.495-4.08-11.293-6.055c-0.8-0.416-1.661-0.714-2.677-1.04
		c-1.514-0.485-1.778-1.175-0.6-1.522c1.179-0.343,1.085-1.122-0.204-1.737c-1.289-0.616-1.102-1.31,0.424-1.547
		c1.526-0.236,1.073-1.126-1.007-1.987c-1.751-0.722-3.489-1.439-5.149-2.125c-2.081-0.861-3.044-2.534-2.081-3.676
		c0.865-1.028,1.498-1.808,1.318-2.273c-0.481-1.244-1.176-2.402-1.954-3.59c-1.241-1.881-1.629-3.362-0.841-3.329
		c0.787,0.032,1.138-0.897,0.791-2.077c-0.212-0.714-0.407-1.428-0.555-2.15c-0.897-4.394-0.856-9.232-2.766-13.117
		c-2.611-5.316-2.102-9.433,0.832-13.121c1.403-1.763,3.187-2.407,3.574-1.808c0.392,0.6,1.403-0.269,2.256-1.941
		c0.856-1.673,1.759-2.521,2.016-1.897s1.645-0.27,3.097-1.991c1.208-1.432,2.439-2.893,3.692-4.378
		c1.452-1.722,2.885-2.428,3.19-1.574c0.311,0.853,1.208-0.168,2.228-2.18c5.104-10.094,13.729-18.106,15.725-29.62
		c0.008-0.049,0.106-0.086,0.216-0.118c0.135-0.041,0.734,0.379,1.335,0.942c0.6,0.562,1.599-0.734,2.231-2.897
		c3.525-12.117,6.936-23.852,10.416-35.813c0.629-2.163,2.293-5.341,3.615-7.165c6.536-8.979,8.209-15.063,8.434-18.649
		c0.143-2.248-1.987-5.153-3.187-7.062c-2.362-3.762-1.856-7.776-2.163-11.706c-0.416-5.271,0.168-10.62,0.608-15.862
		c0.188-2.244,0.534-3.958,1.012-3.741c0.474,0.216,0.457-1.416,0.498-3.668c0.167-9.657,8.706-12.102,15.973-15.137
		c2.081-0.869,5.316-2.204,7.3-2.832c1.252-0.396,2.436-0.873,3.354-1.656c2.591-2.207,4.488-5.243,6.617-7.98
		c1.946-2.501,4.293-4.35,6.879-6.263c4.419-3.264,7.598-8.164,11.539-12.13c3.271-3.288,6.658-6.561,10.42-9.241
		c4.79-3.407,5.814-9.878,2.436-15.043C532.355,246.691,529.156,241.407,526.834,235.822z)"},
    { "Fish",
        R"(M522.697,299.679c13.88-10.49,25.688-19.413,38.038-28.744c-1.648-2.281-2.497-4.243-3.979-5.374
		c-26.703-20.388-53.346-40.865-80.294-60.931c-26.104-19.437-56.994-25.949-88.075-32.102c-5.618-1.114-10.882-5.141-15.77-8.588
		c-12.089-8.535-23.321-18.434-36.01-25.883c-5.459-3.207-13.676-1.706-22.428-2.509c-0.098,0.404-0.804,3.256-1.726,6.989
		c-4.035-2.469-7.446-4.554-11.359-6.948c-0.244,4.247-0.452,7.833-0.722,12.497c-3.582-0.661-6.504-1.204-9.775-1.807
		c-1.311,4.088-2.636,8.229-4.224,13.17c-4.197-1.774-7.491-3.17-11.326-4.794c-1.188,4.056-2.366,8.07-3.807,12.991
		c-4.635-1.428-8.36-2.579-12.791-3.945c-0.849,11.473-7.107,11.844-11.685,9.176c-6.997,2.737-10.902,4.451-14.929,5.806
		c-14.17,4.77-28.531,9.013-42.542,14.202c-17.871,6.622-35.472,13.97-55.786,22.041c1.342,9.065,3.011,20.322,4.753,32.057
		c-15.386,2.427-31.298,1.057-46.087-10.375c-16.071-12.428-32.771-24.284-50.278-34.537c-10.865-6.361-23.745-9.278-35.72-13.742
		c-0.567,1.232-1.134,2.464-1.701,3.692c2.57,6.471,6.173,12.742,7.479,19.458c2.326,11.958,3.786,24.149,4.61,36.312
		c0.722,10.669-0.763,21.51,0.225,32.134c1.673,18.014-2.375,34.569-10.025,50.579c-1.75,3.664-3.345,7.401-6.765,15.011
		c11.481-2.216,20.714-2.183,28.201-5.781c21.269-10.225,42.95-20.277,62.391-33.46c27.14-18.401,52.22-7.903,78.079,0.946
		c0.763,0.262,1.044,1.926,2.497,4.839c-25.296,1.755-23.097,19.27-22.334,36.622c0.318,7.263-1.604,14.619-2.676,23.31
		c15.369,0.212,29.16-6.933,42.656-14.606c6.32-3.595,11.473-9.69,20.123-5.929c1.604,0.697,4.924-0.959,6.671-2.443
		c10.273-8.74,20.11-6.59,31.877-1.89c30.914,12.342,63.416,15.9,96.598,12.954c3.149-0.277,6.345-0.037,13.056-0.037
		c-15.263,6.822-12.57,15.644-8.801,25.872c4.831,13.112,7.956,26.854,12.392,42.33c3.391-3.844,7.479-6.761,9.184-10.706
		c7.716-17.83,15.549-35.692,21.633-54.105c2.333-7.066,4.859-9.755,11.954-10.987c37.969-6.581,75.994-13.007,113.665-21.064
		c13.97-2.987,27.111-10.062,40.424-15.749c2.555-1.094,4.158-4.419,6.198-6.716c-2.763-2.109-5.305-5.594-8.327-6.075
		C545.79,301.29,535.933,300.849,522.697,299.679z)"},
    { "Turtle",
        R"(M565.399,246.059c0.196-6.079-4.357-10.624-10.514-10.812c-2.342-0.069-4.696-0.143-7.034,0.012
		c-1.824,0.123-3.619,0.094-5.394-0.041c-0.412-0.094-0.837-0.135-1.273-0.11c-6.581-0.689-12.885-2.803-19.322-4.529
		c-5.117-1.371-10.764-0.795-16.174-1.057c-0.697-0.033-1.42,0.27-2.105,0.188c-5.177-0.608-9.559,1.163-13.284,4.582
		c-2.488,2.285-5.373,3.093-8.617,3.093c-7.221,0-14.442,0-21.562,0c-1.714-5.443-3.366-10.881-5.055-16.3
		c-2.134-9.311-5.316-18.307-9.466-26.81c-4.876-11.208-11.09-21.677-19.458-31.008c-9.249-10.311-20.416-17.887-32.203-24.823
		c-8.915-5.247-17.784-10.608-26.341-16.41c-6.675-4.528-12.889-9.747-19.208-14.786c-6.259-4.994-13.692-7.503-21.024-10.212
		c-7.393-2.729-14.88-5.214-22.326-7.813c-6.426-2.244-12.754-4.839-19.302-6.626c-4.092-1.118-8.568-1.644-12.783-1.314
		c-4.524,0.355-9.017,1.808-13.407,3.158c-12.056,3.705-24.088,7.503-36.063,11.46c-11.995,3.962-23.978,3.142-36.251,1.326
		c-11.501-1.701-23.105-2.052-34.851-0.535c-11.539,1.489-20.331,6.92-28.258,14.835c-4.243,4.235-8.743,8.111-13.472,11.673
		c-9.078,4.664-17.858,9.768-26.01,15.704c-3.46,1.616-7.062,2.95-10.588,4.435c-7.833,3.296-12.317,9.874-16.487,16.777
		c-6.414,10.612-11.346,21.938-14.872,33.75c-2.774,9.303-4.304,18.976-6.446,28.47c-1.448,6.435-2.88,12.914-9.996,15.786
		c-2.603,1.053-5.182,2.17-7.789,3.219c-0.559,0.225-1.098,0.478-1.62,0.743c-1.64,0.571-3.28,1.138-4.921,1.709
		c-0.449-0.024-0.901-0.049-1.35-0.069c-1.102-0.065-2.163,0.126-3.15,0.498c-0.212,0.041-0.424,0.094-0.637,0.151
		c-4.675,1.285-7.927,6.52-6.41,11.29c4.288,13.472,11.123,25.944,20,36.748c0.037,0.062,0.069,0.122,0.106,0.184
		c2.982,5.039,7.438,9.355,11.775,13.415c6.202,5.81,5.977,5.455,3.603,13.542c-2.138,7.274-4.941,14.431-5.177,22.239
		c-0.196,6.353,1.909,12.069,3.696,17.937c0.094,0.306,0.188,0.615,0.282,0.922c0,0.021,0.004,0.045,0.008,0.065
		c0.135,0.979,0.453,1.954,0.89,2.904c0.996,3.313,1.954,6.639,2.726,10.005c0.559,2.447,0.224,4.622-0.755,6.531
		c-0.625,0.67-1.146,1.429-1.55,2.253c-0.637,0.722-1.367,1.399-2.187,2.023c-4.549,3.468-7.654,7.503-7.405,13.668
		c0.13,3.223,1.897,5.218,3.852,7.226c2.485,2.55,5.259,4.826,7.634,7.467c3.717,4.137,8.445,5.94,13.741,5.871
		c6.711-0.086,12.448,2.248,17.516,6.263c3.733,2.958,7.401,2.949,11.102,0.571c3.798-2.444,7.417-2.469,11.62-1.004
		c3.17,1.105,6.72,1.118,10.098,1.619c2.24,0.335,3.888-0.347,4.88-1.615c0.241-0.249,0.453-0.522,0.636-0.832
		c0.661-1.126,0.767-2.444,0.412-3.627c-0.082-0.751-0.257-1.526-0.547-2.313c-0.6-1.633-1.424-3.19-2.044-4.807
		c-0.122-0.914-0.383-1.804-0.771-2.647c-0.478-2.375-0.375-4.655,0.143-6.843c2.261-3.35,4.08-6.984,5.443-10.783
		c3.905-7.507,3.644-15.562,2.733-23.811c-0.098-0.873-0.204-1.742-0.297-2.469c2.533,0.453,5.067,0.901,7.601,1.354
		c5.529,1.771,11.126,3.558,16.781,5.137c3.464,0.967,6.969,1.787,10.498,2.514c6.324,1.66,12.791,2.7,19.323,3.007
		c1.84,0.208,3.68,0.412,5.516,0.628c6.916,0.816,13.823,1.812,20.767,2.289c7.605,0.526,15.243,0.624,22.869,0.775
		c5.74,0.114,5.577,0.098,7.993,5.169c0.494,1.032,1.048,2.04,1.624,3.031c1.188,2.514,2.33,5.047,3.436,7.602
		c1.081,2.664,2.122,5.341,3.113,8.037c0.29,3.717-0.596,7.394-3.239,10.927c-2.848,3.807-5.692,7.621-8.47,11.477
		c-4.806,6.659-6.124,14.027-4.072,21.935c0.334,1.297,1.293,2.55,2.256,3.541c5.304,5.455,10.677,10.845,16.083,16.202
		c0.767,0.763,1.758,1.42,2.766,1.791c4.594,1.693,9.38,2.876,12.403,7.393c0.64,0.959,2.697,1.249,4.129,1.339
		c0.171,0.012,0.338,0.028,0.506,0.04c1.313,0.779,2.909,0.983,4.402,0.661c1.995,0.449,3.954,1.081,5.981,1.897
		c3.015,1.216,7.609,2.203,11.077-1.021c1.146-1.064,2.811-1.546,4.096-2.492c1.22-0.897,2.448-1.384,3.725-1.302
		c0.853,0.396,1.738,0.706,2.636,0.935c7.947,4.084,16.111-1.347,20.396-5.811c1.865-1.941,2.077-5.765,0.429-8.486
		c-1.322-2.183-2.737-4.309-4.166-6.43c-1.391-2.497-2.782-4.994-4.174-7.487c-0.611-2.713,0.307-5.169,2.819-7.923
		c7.189-7.858,13.501-16.304,18.548-25.741c2.469-4.614,2.99-9.563,3.529-14.479c0.146-1.334,0.265-2.673,0.375-4.011
		c0.307-1.538,0.408-3.084,0.327-4.606c0.257-4.309,0.411-8.625,0.628-12.934c0.081-1.656,0.682-2.199,2.374-2.366
		c9.038-0.901,18.111-1.632,27.071-3.064c10.608-1.697,21.139-3.921,31.648-6.181c3.599-0.775,7.07-2.224,10.515-3.582
		c2.611-1.032,4.528-0.845,5.63,0.596c0.188,1.465,0.56,2.88,1.094,4.255c0.44,6.969,0.562,13.97,1.281,20.906
		c0.616,5.978,1.86,11.897,3.007,17.805c0.253,1.306,1.008,2.746,1.971,3.647c1.652,1.551,3.011,3.256,4.17,5.067
		c0.432,0.768,0.877,1.53,1.334,2.285c1.636,3.056,2.855,6.332,4.051,9.633c0.359,1.889,0.976,3.68,1.918,5.279
		c0.151,0.392,0.302,0.783,0.465,1.171c2.317,5.614,7.34,6.178,13.917,5.753c2.669-0.171,5.41-0.143,8.042,0.273
		c3.06,0.481,5.998,1.718,9.054,2.216c3.986,0.652,7.731-0.44,11.42-2.073c0.583-0.257,1.228-0.469,1.897-0.62
		c0.534,0.168,1.093,0.282,1.668,0.331c1.562,0.139,3.113,0.114,4.644-0.041c2.28,0.098,4.394-0.425,6.373-1.502
		c2.031-0.767,3.97-1.786,5.772-3.027c1.339-0.367,2.746-0.506,4.276-0.403c3.554,0.236,7.132,0.057,10.702,0.053
		c0.566,0,1.109-0.065,1.636-0.18c0.975-0.102,1.877-0.514,2.647-1.13c2.893-1.95,4.293-5.757,2.521-9.221
		c-0.27-0.53-0.571-1.057-0.885-1.579c-0.421-1.354-1.253-2.591-2.407-3.342c-0.882-1.02-1.845-1.95-2.877-2.717
		c-6.642-4.929-11.722-10.775-13.496-19.009c-0.657-3.048-1.081-6.186-1.24-9.299c-0.314-6.083-0.649-12.187-0.478-18.266
		c0.326-11.542-2.313-21.999-8.642-31.375c-2.023-3.84-4.524-7.405-7.471-10.588c-0.979-1.546-1.934-3.093-2.921-4.586
		c0.832-0.905,1.66-1.815,2.488-2.722c3.668-2.79,8.124-3.378,12.461-3.855c5.618-0.62,10-3.431,14.3-6.564
		c2.697-1.963,4.97-4.549,7.781-6.291c5.218-3.235,11.236-4.166,17.209-5.092c10.755-1.673,21.375-3.688,30.556-10.185
		c5.01-3.541,7.58-8.747,9.225-14.198c2.023-6.69,3.149-13.668,4.438-20.567c0.033-0.179,0.062-0.362,0.09-0.546
		c0.245-0.584,0.453-1.188,0.629-1.812l0.546-4.068c0-0.763-0.057-1.521-0.171-2.268
		C565.33,247.536,565.375,246.798,565.399,246.059z)"},
};

/*
* Helper function that gets the all of the t-values that need to be added between
* one t-value on a path to the next, from a sorted queue |tValuesToAdd|. Converts
* the value from its proportion across the whole line to it's proportion relative
* to the current segment.
*/
std::vector<SkScalar> getTValuesForSegment(std::deque<float>* tValuesToAdd, float t, float tNext) {
    std::vector<SkScalar> tVector;
    while (!tValuesToAdd->empty() && tValuesToAdd->front() > t && tValuesToAdd->front() < tNext) {
        SkScalar total_t = tValuesToAdd->front();
        tValuesToAdd->pop_front();
        SkScalar relative_t = (total_t - t) / (tNext - t);
        tVector.push_back(relative_t);
    }
    return tVector;
}

/*
* Helper function that takes a vector of t-values and chops a cubic at those correct
* values, added it to the path |out|.
*/
void addSegmentsFromTValues(const SkPoint cubic_pts[4], std::vector<SkScalar> t_values, SkPath* out) {
    const size_t arr_size = t_values.size();
    const int dst_size = (3*arr_size) + 4;
    std::vector<SkPoint> split_pts(dst_size);
    SkChopCubicAt(cubic_pts, split_pts.data(), t_values.data(), arr_size);

    for (size_t i = 0; i < arr_size + 1; i++) {
        out->cubicTo(split_pts[(i*3)+1], split_pts[(i*3)+2], split_pts[(i*3)+3]);
    }
}

/*
* Helper function that given a path, it's t-values (sorted), and t-values to add
* (sorted), returns a new path that is all cubic beziers, with verbs at each of
* those t-values.
*/
bool createPathFromTValues(const SkPath& in, std::deque<float> tValuesToAdd, std::vector<float> tValues, SkPath* out) {
    SkPath::Iter iter(in, false);
    bool fBreak = false;

    // Only increment if we draw on the path.
    size_t t_value_idx = 0;

    for (;;) {
        if (fBreak) break;
        bool needToSplit = false;
        SkPoint pts[4];
        SkPath::Verb verb = iter.next(pts);

        // The last t-value is always the end of the path (when t=1).
        if (t_value_idx >= tValues.size() - 1) {
            break;
        }
        // t and tNext are the start and end of the current segment.
        float t = tValues[t_value_idx];
        float tNext = tValues[t_value_idx+1];

        // Check if current tValueToAdd is on this current segment.
        if (!tValuesToAdd.empty() && tValuesToAdd.front() > t && tValuesToAdd.front() < tNext) {
            needToSplit = true;
        }

        switch (verb) {
            case SkPath::kMove_Verb:
               // Only supports one contour currently.
               out->moveTo(pts[0]);
            break;
            case SkPath::kLine_Verb: {
                t_value_idx++;
                SkPoint pt1, pt2;
                pt1 = pts[0]*(1.0f / 3.0f) + pts[1]*(2.0f / 3.0f);
                pt2 = pts[0]*(2.0f / 3.0f) + pts[1]*(1.0f / 3.0f);
                if (!needToSplit) {
                    out->cubicTo(pt1, pt2, pts[1]);
                } else {
                    std::vector<SkScalar> tVector = getTValuesForSegment(&tValuesToAdd, t, tNext);
                    const SkPoint cubic_pts[4] = {pts[0], pt1, pt2, pts[1]};
                    addSegmentsFromTValues(cubic_pts, tVector, out);
                }
                break;
            }
            case SkPath::kQuad_Verb: {
                t_value_idx++;
                SkPoint pt1, pt2;
                pt1 = pts[0] + (pts[1]-pts[0])*(2.0f / 3.0f);
                pt2 = pts[2] + (pts[1]-pts[2])*(2.0f / 3.0f);
                if (!needToSplit) {
                    out->cubicTo(pt1, pt2, pts[2]);
                } else {
                    std::vector<SkScalar> tVector = getTValuesForSegment(&tValuesToAdd, t, tNext);
                    const SkPoint cubic_pts[4] = {pts[0], pt1, pt2, pts[2]};
                    addSegmentsFromTValues(cubic_pts, tVector, out);
                }
                break;
            }
            case SkPath::kCubic_Verb:
                t_value_idx++;
                if (!needToSplit) {
                    out->cubicTo(pts[1], pts[2], pts[3]);
                } else {
                    std::vector<SkScalar> tVector = getTValuesForSegment(&tValuesToAdd, t, tNext);
                    addSegmentsFromTValues(pts, tVector, out);
                }
                break;
            case SkPath::kConic_Verb:
                // Conic not yet supported.
                return false;
            case SkPath::kClose_Verb:
                // Close not yet supported.
                out->close();
                break;
            case SkPath::kDone_Verb:
                fBreak = true;
        }
    }
    return true;
}

/*
* Helper function to get the total lengths the verbs take of a path and put it
* into a vector.
*/
std::vector<SkScalar> getTValues(const SkPath& path) {
    std::vector<SkScalar> tValues;
    SkPathMeasure measure(path, false);
    SkScalar length = measure.getLength();
    if (length <= 0) {
        SkDebugf("Length of path is 0.\n");
        return tValues;
    }
    const SkContourMeasure* cmeasure = measure.currentMeasure();
    tValues.push_back(0.0f);
    for (const auto vmeasure: *cmeasure) {
        tValues.push_back(vmeasure.fDistance / length);
    }

    if (measure.nextContour()) {
        SkDebugf("Path has more than 1 contour.\n");
        return {};
    }
    return tValues;
}

/*
* Helper function that creates a deque from a sorted vector |original| and adds
* values from a sorted vector |additional| in sorted order.
*/
std::deque<float> getTValuesToAdd(std::vector<SkScalar> original, std::vector<SkScalar> additional) {
    std::deque<float> tValuesToAdd;
    size_t i = 0, j = 0;
    while (i < original.size() && j < additional.size()) {
        if (additional[j] < original[i]) {
            tValuesToAdd.push_back(additional[j]);
            j++;
        } else if (additional[j] > original[i]) {
            i++;
        } else { // additional[j] == original[i]
            i++;
            j++;
        }
    }
    while (j < additional.size()) {
        tValuesToAdd.push_back(additional[j]);
        j++;
    }
    return tValuesToAdd;
}

/*
* Extension to SkPath::Interpolate function that takes two arbitrary SkPaths.
*
* The current functionality of SkPath::Interpolate requires that the two paths
* have identical verbs (same number of verbs and verb types). This function does
* preprocessing on the two paths to create two new paths that fit this requirement
* without modifying the original path.
*
* The function uses a list of t-values to determine where to place points along
* the path, and adds more of these points based on the other paths values. Then all
* verbs are converted to cubic functions. When t-values need to be added, the cubic
* is chopped at the correct positions in accordance to the t-values.
*
* TODO: Add support for multiple contours, conic verbs, and close verbs.
* TODO: Fix phrasing as we don't use actual t-values of the curves just proportional
* distances(?)
*/
bool generalInterpolate(const SkPath& beginning, const SkPath& ending, SkScalar weight, SkPath* out) {
    // Use existing path interpolation if possible.
    if (beginning.isInterpolatable(ending)) {
        return beginning.interpolate(ending, weight, out);
    }

    // TODO: Check if isValid()?
    if (beginning.isEmpty() || !beginning.isFinite() || ending.isEmpty() || !ending.isFinite()) {
        return false;
    }

    // New paths to store the transformed paths.
    SkPath beginningCubic;
    SkPath endingCubic;
    beginningCubic.reset();
    endingCubic.reset();

    // Append the total distances up to each verb in the path into a vector.
    std::vector<SkScalar> tValues1 = getTValues(beginning);
    std::vector<SkScalar> tValues2 = getTValues(ending);
    if (tValues1.empty() || tValues2.empty()) {
        return false;
    }

    // The t-values to add for the respective paths from the other.
    std::deque<float> tValuesToAdd1 = getTValuesToAdd(tValues1, tValues2);
    std::deque<float> tValuesToAdd2 = getTValuesToAdd(tValues2, tValues1);

    // Form the cubic versions of each path with the new points along it.
    createPathFromTValues(beginning, tValuesToAdd1, tValues1, &beginningCubic);
    createPathFromTValues(ending, tValuesToAdd2, tValues2, &endingCubic);

    return beginningCubic.interpolate(endingCubic, weight, out);
}

class PathLerpSlide final : public Slide {
public:
    PathLerpSlide()
        : fTimeMapper({0.5f, 0}, {0.5f, 1}) {
        fName = "PathLerp";
    }

private:
    void load(SkScalar w, SkScalar h) override {
        fSize = {w, h};

        this->updateAnimatingPaths();
    }

    void resize(SkScalar w, SkScalar h) override { fSize = {w, h}; }

    void draw(SkCanvas* canvas) override {
        SkPaint path_paint;
        path_paint.setColor(0xff424242);
        path_paint.setStyle(SkPaint::kStroke_Style);
        path_paint.setAntiAlias(true);
        path_paint.setStrokeCap(SkPaint::kRound_Cap);
        path_paint.setStrokeWidth(10 / fPathTransform.getScaleX());

        SkAutoCanvasRestore acr(canvas, true);
        canvas->concat(fPathTransform);
        canvas->drawPath(fInterpolatedPath, path_paint);

        const auto draw_vertices = [this](SkCanvas* canvas, const SkPath& path, float opacity) {
            SkPaint vertex_paint, ctrl_paint;
            vertex_paint.setColor4f({1, 0, 0, opacity});
            vertex_paint.setAntiAlias(true);
            ctrl_paint.setColor4f({0, 0, 1, opacity});
            ctrl_paint.setAntiAlias(true);

            const float vertex_radius = 6 / fPathTransform.getScaleX(),
                          ctrl_radius = 4 / fPathTransform.getScaleX();

            for (const auto [verb, pts, weights] : SkPathPriv::Iterate(path)) {
                switch (verb) {
                    case SkPathVerb::kMove: // pts: [ vertex ]
                        canvas->drawCircle(pts[0], vertex_radius, vertex_paint);
                        break;
                    case SkPathVerb::kLine: // pts: [ prev_vertex, vertex ]
                        canvas->drawCircle(pts[1], vertex_radius, vertex_paint);
                        break;
                    case SkPathVerb::kQuad: // pts: [ prev_vertex, ctrl, vertex ]
                        canvas->drawCircle(pts[1], ctrl_radius, ctrl_paint);
                        canvas->drawCircle(pts[2], vertex_radius, vertex_paint);
                        break;
                    case SkPathVerb::kCubic: // pts: [ prev_vertex, ctrl0, ctrl1, vertex ]
                        canvas->drawCircle(pts[1], ctrl_radius, ctrl_paint);
                        canvas->drawCircle(pts[2], ctrl_radius, ctrl_paint);
                        canvas->drawCircle(pts[3], vertex_radius, vertex_paint);
                        break;
                    case SkPathVerb::kConic: // pts: [ prev_vertex, ctrl, vertex ]
                        canvas->drawCircle(pts[1], ctrl_radius, ctrl_paint);
                        canvas->drawCircle(pts[2], vertex_radius, vertex_paint);
                        break;
                    case SkPathVerb::kClose: // pts: []
                        break;
                }
            }
        };

        if (fShowVertices) {
            draw_vertices(canvas, fInterpolatedPath, 1);

            // also show the input paths & vertices
            path_paint.setAlphaf(.15f);
            path_paint.setStrokeWidth(3 / fPathTransform.getScaleX());

            canvas->drawPath(fPaths.first , path_paint);
            canvas->drawPath(fPaths.second, path_paint);
            draw_vertices(canvas, fPaths.first , .15f);
            draw_vertices(canvas, fPaths.second, .15f);
        }

        this->drawControls();
    }

    bool animate(double nanos) override {
        if (!fTimeBase) {
            fTimeBase = nanos;
        }

        if (fDraggingProgress) {
            // When progress is controlled by dragging the slidebar, adjust the time base such
            // that the animation continues seamlessly when the slider is released.

            // what the (signed) progress should be according to the clock
            const float clock_progress_signed =
                std::fmod((nanos - fTimeBase)*0.000000001*fAnimationSpeed, 2) - 1;
            // what progress should be to match the slidebar
            const float clock_progress_adjusted =
                std::copysign(fCurrentProgress, clock_progress_signed);
            // what the clock should be to match the slidebar
            const float nanos_adjusted = (clock_progress_adjusted + 1)*1000000000/fAnimationSpeed;

            fTimeBase = nanos - nanos_adjusted;
        } else {
            // Oscillating between 0..1
            fCurrentProgress =
                    std::abs((std::fmod((nanos - fTimeBase)*0.000000001*fAnimationSpeed, 2) - 1));
        }

        // Interpolate with easing
        // TODO: generate the synthetic paths once, in updateAnimatingPaths(), then use
        //       regular interpolation here.
        return generalInterpolate(fPaths.first,
                                  fPaths.second,
                                  fTimeMapper.computeYFromX(fCurrentProgress),
                                  &fInterpolatedPath);
    }

    bool onChar(SkUnichar c) override {
        switch (c) {
            case 'v':
                fShowVertices = !fShowVertices;
                return true;
            default:
                return false;
        }
    }

    bool onMouse(SkScalar x, SkScalar y, skui::InputState state, skui::ModifierKey) override {
        // Show the progress slider when hovering the bottom third.
        fShowSlider = y > fSize.height() * .66f;

        return false;
    }

    void updateAnimatingPaths() {
        SkPath p0, p1;
        SkAssertResult(SkParsePath::FromSVGString(fSelectedPaths[0]->fSVGString, &p0));
        SkAssertResult(SkParsePath::FromSVGString(fSelectedPaths[1]->fSVGString, &p1));

        const SkRect b0 = p0.computeTightBounds(),
                     b1 = p1.computeTightBounds();

        // Transform all paths to a normalized size, such that they occupy roughly the same space.
        static constexpr SkRect kNormRect = {0, 0, 512, 512};

        fPaths = {
            p0.transform(SkMatrix::MakeRectToRect(b0, kNormRect, SkMatrix::kCenter_ScaleToFit)),
            p1.transform(SkMatrix::MakeRectToRect(b1, kNormRect, SkMatrix::kCenter_ScaleToFit)),
        };


        // Scale and center such that the path animation fills 90% of the view.
        SkRect bounds = p0.computeTightBounds();
        bounds.join(p1.computeTightBounds());

        const SkRect dst_rect = SkRect::MakeSize(fSize)
            .makeInset(fSize.width() * .05f, fSize.height() * .05f);
        fPathTransform =
            SkMatrix::MakeRectToRect(kNormRect, dst_rect, SkMatrix::kCenter_ScaleToFit);
    }

    void drawControls() {
        // path controls
        if (ImGui::Begin("Path Options")) {
            for (size_t i = 0; i < 2; ++i) {
                const SkString label = SkStringPrintf("Path %zu", i + 1);
                if (ImGui::BeginCombo(label.c_str(), fSelectedPaths[i]->fName)) {
                    for (const auto& path_desc : gSamplePaths) {
                        const auto is_selected = (fSelectedPaths[i] == &path_desc);
                        if (ImGui::Selectable(path_desc.fName) && !is_selected) {
                            fSelectedPaths[i] = &path_desc;
                            this->updateAnimatingPaths();
                        }
                        if (is_selected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }
            }

            ImGui::Checkbox("Show vertices", &fShowVertices);

        }
        ImGui::End();

        if (!fShowSlider) {
            return;
        }

        // progress slider
        ImGui::SetNextWindowBgAlpha(.75f);
        if (ImGui::Begin("Progress Slider", nullptr, ImGuiWindowFlags_NoDecoration |
                                                     ImGuiWindowFlags_NoResize |
                                                     ImGuiWindowFlags_NoMove |
                                                     ImGuiWindowFlags_NoSavedSettings |
                                                     ImGuiWindowFlags_NoFocusOnAppearing |
                                                     ImGuiWindowFlags_NoNav)) {
            static constexpr float kSliderHeight = 100;
            ImGui::SetWindowPos({0, fSize.height() - kSliderHeight});
            ImGui::SetWindowSize({fSize.width(), kSliderHeight});

            ImGui::PushItemWidth(-1);
            ImGui::SliderFloat("", &fCurrentProgress, 0, 1, nullptr, ImGuiSliderFlags_NoInput);
            fDraggingProgress = ImGui::IsItemActive();
            ImGui::PopItemWidth();
        }
        ImGui::End();
    }

    SkSize                    fSize = {0,0};
    std::pair<SkPath, SkPath> fPaths;        // currently morphing paths
    SkPath                    fInterpolatedPath;
    SkMatrix                  fPathTransform = SkMatrix::I();

    float                     fAnimationSpeed   = 1.f;
    double                    fTimeBase         = 0;
    const SkCubicMap          fTimeMapper;   // for animation easing
    float                     fCurrentProgress  = 0; // Interpolation progress [0..1]

    // UI stuff
    const PathDesc*           fSelectedPaths[2] = {&gSamplePaths[0], &gSamplePaths[1]};
    bool                      fDraggingProgress = false;
    bool                      fShowVertices     = false;
    bool                      fShowSlider       = false;
};

}  // namespace

DEF_SLIDE(return new PathLerpSlide();)


