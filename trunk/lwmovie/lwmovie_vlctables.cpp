#include "lwmovie_vlc.hpp"


lwmovie::vlc::lwmVlcValue8 lwmovie::vlc::coded_block_pattern[512] = 
{
	{lwmovie::vlc::UERROR8, 0}, {lwmovie::vlc::UERROR8, 0},
	{39, 9}, {27, 9}, {59, 9}, {55, 9}, {47, 9}, {31, 9},
	{58, 8}, {58, 8}, {54, 8}, {54, 8}, {46, 8}, {46, 8}, {30, 8}, {30, 8},
	{57, 8}, {57, 8}, {53, 8}, {53, 8}, {45, 8}, {45, 8}, {29, 8}, {29, 8},
	{38, 8}, {38, 8}, {26, 8}, {26, 8}, {37, 8}, {37, 8}, {25, 8}, {25, 8},
	{43, 8}, {43, 8}, {23, 8}, {23, 8}, {51, 8}, {51, 8}, {15, 8}, {15, 8},
	{42, 8}, {42, 8}, {22, 8}, {22, 8}, {50, 8}, {50, 8}, {14, 8}, {14, 8},
	{41, 8}, {41, 8}, {21, 8}, {21, 8}, {49, 8}, {49, 8}, {13, 8}, {13, 8},
	{35, 8}, {35, 8}, {19, 8}, {19, 8}, {11, 8}, {11, 8}, {7, 8}, {7, 8},
	{34, 7}, {34, 7}, {34, 7}, {34, 7}, {18, 7}, {18, 7}, {18, 7}, {18, 7},
	{10, 7}, {10, 7}, {10, 7}, {10, 7}, {6, 7}, {6, 7}, {6, 7}, {6, 7}, 
	{33, 7}, {33, 7}, {33, 7}, {33, 7}, {17, 7}, {17, 7}, {17, 7}, {17, 7}, 
	{9, 7}, {9, 7}, {9, 7}, {9, 7}, {5, 7}, {5, 7}, {5, 7}, {5, 7}, 
	{63, 6}, {63, 6}, {63, 6}, {63, 6}, {63, 6}, {63, 6}, {63, 6}, {63, 6}, 
	{3, 6}, {3, 6}, {3, 6}, {3, 6}, {3, 6}, {3, 6}, {3, 6}, {3, 6}, 
	{36, 6}, {36, 6}, {36, 6}, {36, 6}, {36, 6}, {36, 6}, {36, 6}, {36, 6}, 
	{24, 6}, {24, 6}, {24, 6}, {24, 6}, {24, 6}, {24, 6}, {24, 6}, {24, 6}, 
	{62, 5}, {62, 5}, {62, 5}, {62, 5}, {62, 5}, {62, 5}, {62, 5}, {62, 5},
	{62, 5}, {62, 5}, {62, 5}, {62, 5}, {62, 5}, {62, 5}, {62, 5}, {62, 5},
	{2, 5}, {2, 5}, {2, 5}, {2, 5}, {2, 5}, {2, 5}, {2, 5}, {2, 5}, 
	{2, 5}, {2, 5}, {2, 5}, {2, 5}, {2, 5}, {2, 5}, {2, 5}, {2, 5}, 
	{61, 5}, {61, 5}, {61, 5}, {61, 5}, {61, 5}, {61, 5}, {61, 5}, {61, 5}, 
	{61, 5}, {61, 5}, {61, 5}, {61, 5}, {61, 5}, {61, 5}, {61, 5}, {61, 5}, 
	{1, 5}, {1, 5}, {1, 5}, {1, 5}, {1, 5}, {1, 5}, {1, 5}, {1, 5}, 
	{1, 5}, {1, 5}, {1, 5}, {1, 5}, {1, 5}, {1, 5}, {1, 5}, {1, 5}, 
	{56, 5}, {56, 5}, {56, 5}, {56, 5}, {56, 5}, {56, 5}, {56, 5}, {56, 5}, 
	{56, 5}, {56, 5}, {56, 5}, {56, 5}, {56, 5}, {56, 5}, {56, 5}, {56, 5}, 
	{52, 5}, {52, 5}, {52, 5}, {52, 5}, {52, 5}, {52, 5}, {52, 5}, {52, 5}, 
	{52, 5}, {52, 5}, {52, 5}, {52, 5}, {52, 5}, {52, 5}, {52, 5}, {52, 5}, 
	{44, 5}, {44, 5}, {44, 5}, {44, 5}, {44, 5}, {44, 5}, {44, 5}, {44, 5}, 
	{44, 5}, {44, 5}, {44, 5}, {44, 5}, {44, 5}, {44, 5}, {44, 5}, {44, 5}, 
	{28, 5}, {28, 5}, {28, 5}, {28, 5}, {28, 5}, {28, 5}, {28, 5}, {28, 5}, 
	{28, 5}, {28, 5}, {28, 5}, {28, 5}, {28, 5}, {28, 5}, {28, 5}, {28, 5}, 
	{40, 5}, {40, 5}, {40, 5}, {40, 5}, {40, 5}, {40, 5}, {40, 5}, {40, 5}, 
	{40, 5}, {40, 5}, {40, 5}, {40, 5}, {40, 5}, {40, 5}, {40, 5}, {40, 5}, 
	{20, 5}, {20, 5}, {20, 5}, {20, 5}, {20, 5}, {20, 5}, {20, 5}, {20, 5}, 
	{20, 5}, {20, 5}, {20, 5}, {20, 5}, {20, 5}, {20, 5}, {20, 5}, {20, 5}, 
	{48, 5}, {48, 5}, {48, 5}, {48, 5}, {48, 5}, {48, 5}, {48, 5}, {48, 5}, 
	{48, 5}, {48, 5}, {48, 5}, {48, 5}, {48, 5}, {48, 5}, {48, 5}, {48, 5}, 
	{12, 5}, {12, 5}, {12, 5}, {12, 5}, {12, 5}, {12, 5}, {12, 5}, {12, 5}, 
	{12, 5}, {12, 5}, {12, 5}, {12, 5}, {12, 5}, {12, 5}, {12, 5}, {12, 5}, 
	{32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, 
	{32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, 
	{32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, 
	{32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, 
	{16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, 
	{16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, 
	{16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, 
	{16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, 
	{8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, 
	{8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, 
	{8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, 
	{8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4},
	{4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4},
	{4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, 
	{4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, 
	{4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4},
	{60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, 
	{60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, 
	{60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, 
	{60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, 
	{60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, 
	{60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, 
	{60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, 
	{60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}
};

/* Decoding tables for dct_dc_size_luminance */
lwmovie::vlc::lwmVlcValue8 lwmovie::vlc::dct_dc_size_luminance[32] =
{   {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, 
	{2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, 
	{0, 3}, {0, 3}, {0, 3}, {0, 3}, {3, 3}, {3, 3}, {3, 3}, {3, 3}, 
	{4, 3}, {4, 3}, {4, 3}, {4, 3}, {5, 4}, {5, 4}, {6, 5},
	{lwmovie::vlc::UERROR8, 0}
};

lwmovie::vlc::lwmVlcValue8 lwmovie::vlc::dct_dc_size_luminance1[16] =
{   {7, 6}, {7, 6}, {7, 6}, {7, 6}, {7, 6}, {7, 6}, {7, 6}, {7, 6},
	{8, 7}, {8, 7}, {8, 7}, {8, 7}, {9, 8}, {9, 8}, {10, 9}, {11, 9}
};

/* Decoding table for dct_dc_size_chrominance */
lwmovie::vlc::lwmVlcValue8 lwmovie::vlc::dct_dc_size_chrominance[32] =
{   {0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2}, 
	{1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, 
	{2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, 
	{3, 3}, {3, 3}, {3, 3}, {3, 3}, {4, 4}, {4, 4}, {5, 5},
	{lwmovie::vlc::UERROR8, 0}
};

lwmovie::vlc::lwmVlcValue8 lwmovie::vlc::dct_dc_size_chrominance1[32] =
{   {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, 
	{6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, 
	{7, 7}, {7, 7}, {7, 7}, {7, 7}, {7, 7}, {7, 7}, {7, 7}, {7, 7}, 
	{8, 8}, {8, 8}, {8, 8}, {8, 8}, {9, 9}, {9, 9}, {10, 10}, {11, 10}
};

/* DCT coeff tables. */

lwmUInt16 lwmovie::vlc::dct_coeff_tbl_0[256] =
{
	0xffff, 0xffff, 0xffff, 0xffff, 
	0xffff, 0xffff, 0xffff, 0xffff, 
	0xffff, 0xffff, 0xffff, 0xffff, 
	0xffff, 0xffff, 0xffff, 0xffff, 
	0x052f, 0x051f, 0x050f, 0x04ff, 
	0x183f, 0x402f, 0x3c2f, 0x382f, 
	0x342f, 0x302f, 0x2c2f, 0x7c1f, 
	0x781f, 0x741f, 0x701f, 0x6c1f, 
	0x028e, 0x028e, 0x027e, 0x027e, 
	0x026e, 0x026e, 0x025e, 0x025e, 
	0x024e, 0x024e, 0x023e, 0x023e, 
	0x022e, 0x022e, 0x021e, 0x021e, 
	0x020e, 0x020e, 0x04ee, 0x04ee, 
	0x04de, 0x04de, 0x04ce, 0x04ce, 
	0x04be, 0x04be, 0x04ae, 0x04ae, 
	0x049e, 0x049e, 0x048e, 0x048e, 
	0x01fd, 0x01fd, 0x01fd, 0x01fd, 
	0x01ed, 0x01ed, 0x01ed, 0x01ed, 
	0x01dd, 0x01dd, 0x01dd, 0x01dd, 
	0x01cd, 0x01cd, 0x01cd, 0x01cd, 
	0x01bd, 0x01bd, 0x01bd, 0x01bd, 
	0x01ad, 0x01ad, 0x01ad, 0x01ad, 
	0x019d, 0x019d, 0x019d, 0x019d, 
	0x018d, 0x018d, 0x018d, 0x018d, 
	0x017d, 0x017d, 0x017d, 0x017d, 
	0x016d, 0x016d, 0x016d, 0x016d, 
	0x015d, 0x015d, 0x015d, 0x015d, 
	0x014d, 0x014d, 0x014d, 0x014d, 
	0x013d, 0x013d, 0x013d, 0x013d, 
	0x012d, 0x012d, 0x012d, 0x012d, 
	0x011d, 0x011d, 0x011d, 0x011d, 
	0x010d, 0x010d, 0x010d, 0x010d, 
	0x282c, 0x282c, 0x282c, 0x282c, 
	0x282c, 0x282c, 0x282c, 0x282c, 
	0x242c, 0x242c, 0x242c, 0x242c, 
	0x242c, 0x242c, 0x242c, 0x242c, 
	0x143c, 0x143c, 0x143c, 0x143c, 
	0x143c, 0x143c, 0x143c, 0x143c, 
	0x0c4c, 0x0c4c, 0x0c4c, 0x0c4c, 
	0x0c4c, 0x0c4c, 0x0c4c, 0x0c4c, 
	0x085c, 0x085c, 0x085c, 0x085c, 
	0x085c, 0x085c, 0x085c, 0x085c, 
	0x047c, 0x047c, 0x047c, 0x047c, 
	0x047c, 0x047c, 0x047c, 0x047c, 
	0x046c, 0x046c, 0x046c, 0x046c, 
	0x046c, 0x046c, 0x046c, 0x046c, 
	0x00fc, 0x00fc, 0x00fc, 0x00fc, 
	0x00fc, 0x00fc, 0x00fc, 0x00fc, 
	0x00ec, 0x00ec, 0x00ec, 0x00ec, 
	0x00ec, 0x00ec, 0x00ec, 0x00ec, 
	0x00dc, 0x00dc, 0x00dc, 0x00dc, 
	0x00dc, 0x00dc, 0x00dc, 0x00dc, 
	0x00cc, 0x00cc, 0x00cc, 0x00cc, 
	0x00cc, 0x00cc, 0x00cc, 0x00cc, 
	0x681c, 0x681c, 0x681c, 0x681c, 
	0x681c, 0x681c, 0x681c, 0x681c, 
	0x641c, 0x641c, 0x641c, 0x641c, 
	0x641c, 0x641c, 0x641c, 0x641c, 
	0x601c, 0x601c, 0x601c, 0x601c, 
	0x601c, 0x601c, 0x601c, 0x601c, 
	0x5c1c, 0x5c1c, 0x5c1c, 0x5c1c, 
	0x5c1c, 0x5c1c, 0x5c1c, 0x5c1c, 
	0x581c, 0x581c, 0x581c, 0x581c, 
	0x581c, 0x581c, 0x581c, 0x581c, 
};

lwmUInt16 lwmovie::vlc::dct_coeff_tbl_1[16] = 
{
	0x00bb, 0x202b, 0x103b, 0x00ab, 
	0x084b, 0x1c2b, 0x541b, 0x501b, 
	0x009b, 0x4c1b, 0x481b, 0x045b, 
	0x0c3b, 0x008b, 0x182b, 0x441b, 
};

lwmUInt16 lwmovie::vlc::dct_coeff_tbl_2[4] =
{
	0x4019, 0x1429, 0x0079, 0x0839, 
};

lwmUInt16 lwmovie::vlc::dct_coeff_tbl_3[4] = 
{
	0x0449, 0x3c19, 0x3819, 0x1029, 
};

lwmUInt16 lwmovie::vlc::dct_coeff_next[256] = 
{
	0xffff, 0xffff, 0xffff, 0xffff, 
	0xf7d5, 0xf7d5, 0xf7d5, 0xf7d5, 
	0x0826, 0x0826, 0x2416, 0x2416, 
	0x0046, 0x0046, 0x2016, 0x2016, 
	0x1c15, 0x1c15, 0x1c15, 0x1c15, 
	0x1815, 0x1815, 0x1815, 0x1815, 
	0x0425, 0x0425, 0x0425, 0x0425, 
	0x1415, 0x1415, 0x1415, 0x1415, 
	0x3417, 0x0067, 0x3017, 0x2c17, 
	0x0c27, 0x0437, 0x0057, 0x2817, 
	0x0034, 0x0034, 0x0034, 0x0034, 
	0x0034, 0x0034, 0x0034, 0x0034, 
	0x1014, 0x1014, 0x1014, 0x1014, 
	0x1014, 0x1014, 0x1014, 0x1014, 
	0x0c14, 0x0c14, 0x0c14, 0x0c14, 
	0x0c14, 0x0c14, 0x0c14, 0x0c14, 
	0x0023, 0x0023, 0x0023, 0x0023, 
	0x0023, 0x0023, 0x0023, 0x0023, 
	0x0023, 0x0023, 0x0023, 0x0023, 
	0x0023, 0x0023, 0x0023, 0x0023, 
	0x0813, 0x0813, 0x0813, 0x0813, 
	0x0813, 0x0813, 0x0813, 0x0813, 
	0x0813, 0x0813, 0x0813, 0x0813, 
	0x0813, 0x0813, 0x0813, 0x0813, 
	0x0412, 0x0412, 0x0412, 0x0412, 
	0x0412, 0x0412, 0x0412, 0x0412, 
	0x0412, 0x0412, 0x0412, 0x0412, 
	0x0412, 0x0412, 0x0412, 0x0412, 
	0x0412, 0x0412, 0x0412, 0x0412, 
	0x0412, 0x0412, 0x0412, 0x0412, 
	0x0412, 0x0412, 0x0412, 0x0412, 
	0x0412, 0x0412, 0x0412, 0x0412, 
	0xfbe1, 0xfbe1, 0xfbe1, 0xfbe1, 
	0xfbe1, 0xfbe1, 0xfbe1, 0xfbe1, 
	0xfbe1, 0xfbe1, 0xfbe1, 0xfbe1, 
	0xfbe1, 0xfbe1, 0xfbe1, 0xfbe1, 
	0xfbe1, 0xfbe1, 0xfbe1, 0xfbe1, 
	0xfbe1, 0xfbe1, 0xfbe1, 0xfbe1, 
	0xfbe1, 0xfbe1, 0xfbe1, 0xfbe1, 
	0xfbe1, 0xfbe1, 0xfbe1, 0xfbe1, 
	0xfbe1, 0xfbe1, 0xfbe1, 0xfbe1, 
	0xfbe1, 0xfbe1, 0xfbe1, 0xfbe1, 
	0xfbe1, 0xfbe1, 0xfbe1, 0xfbe1, 
	0xfbe1, 0xfbe1, 0xfbe1, 0xfbe1, 
	0xfbe1, 0xfbe1, 0xfbe1, 0xfbe1, 
	0xfbe1, 0xfbe1, 0xfbe1, 0xfbe1, 
	0xfbe1, 0xfbe1, 0xfbe1, 0xfbe1, 
	0xfbe1, 0xfbe1, 0xfbe1, 0xfbe1, 
	0x0011, 0x0011, 0x0011, 0x0011, 
	0x0011, 0x0011, 0x0011, 0x0011, 
	0x0011, 0x0011, 0x0011, 0x0011, 
	0x0011, 0x0011, 0x0011, 0x0011, 
	0x0011, 0x0011, 0x0011, 0x0011, 
	0x0011, 0x0011, 0x0011, 0x0011, 
	0x0011, 0x0011, 0x0011, 0x0011, 
	0x0011, 0x0011, 0x0011, 0x0011, 
	0x0011, 0x0011, 0x0011, 0x0011, 
	0x0011, 0x0011, 0x0011, 0x0011, 
	0x0011, 0x0011, 0x0011, 0x0011, 
	0x0011, 0x0011, 0x0011, 0x0011, 
	0x0011, 0x0011, 0x0011, 0x0011, 
	0x0011, 0x0011, 0x0011, 0x0011, 
	0x0011, 0x0011, 0x0011, 0x0011, 
	0x0011, 0x0011, 0x0011, 0x0011, 
};

lwmUInt16 lwmovie::vlc::dct_coeff_first[256] = 
{
	0xffff, 0xffff, 0xffff, 0xffff, 
	0xf7d5, 0xf7d5, 0xf7d5, 0xf7d5, 
	0x0826, 0x0826, 0x2416, 0x2416, 
	0x0046, 0x0046, 0x2016, 0x2016, 
	0x1c15, 0x1c15, 0x1c15, 0x1c15, 
	0x1815, 0x1815, 0x1815, 0x1815, 
	0x0425, 0x0425, 0x0425, 0x0425, 
	0x1415, 0x1415, 0x1415, 0x1415, 
	0x3417, 0x0067, 0x3017, 0x2c17, 
	0x0c27, 0x0437, 0x0057, 0x2817, 
	0x0034, 0x0034, 0x0034, 0x0034, 
	0x0034, 0x0034, 0x0034, 0x0034, 
	0x1014, 0x1014, 0x1014, 0x1014, 
	0x1014, 0x1014, 0x1014, 0x1014, 
	0x0c14, 0x0c14, 0x0c14, 0x0c14, 
	0x0c14, 0x0c14, 0x0c14, 0x0c14, 
	0x0023, 0x0023, 0x0023, 0x0023, 
	0x0023, 0x0023, 0x0023, 0x0023, 
	0x0023, 0x0023, 0x0023, 0x0023, 
	0x0023, 0x0023, 0x0023, 0x0023, 
	0x0813, 0x0813, 0x0813, 0x0813, 
	0x0813, 0x0813, 0x0813, 0x0813, 
	0x0813, 0x0813, 0x0813, 0x0813, 
	0x0813, 0x0813, 0x0813, 0x0813, 
	0x0412, 0x0412, 0x0412, 0x0412, 
	0x0412, 0x0412, 0x0412, 0x0412, 
	0x0412, 0x0412, 0x0412, 0x0412, 
	0x0412, 0x0412, 0x0412, 0x0412, 
	0x0412, 0x0412, 0x0412, 0x0412, 
	0x0412, 0x0412, 0x0412, 0x0412, 
	0x0412, 0x0412, 0x0412, 0x0412, 
	0x0412, 0x0412, 0x0412, 0x0412, 
	0x0010, 0x0010, 0x0010, 0x0010, 
	0x0010, 0x0010, 0x0010, 0x0010, 
	0x0010, 0x0010, 0x0010, 0x0010, 
	0x0010, 0x0010, 0x0010, 0x0010, 
	0x0010, 0x0010, 0x0010, 0x0010, 
	0x0010, 0x0010, 0x0010, 0x0010, 
	0x0010, 0x0010, 0x0010, 0x0010, 
	0x0010, 0x0010, 0x0010, 0x0010, 
	0x0010, 0x0010, 0x0010, 0x0010, 
	0x0010, 0x0010, 0x0010, 0x0010, 
	0x0010, 0x0010, 0x0010, 0x0010, 
	0x0010, 0x0010, 0x0010, 0x0010, 
	0x0010, 0x0010, 0x0010, 0x0010, 
	0x0010, 0x0010, 0x0010, 0x0010, 
	0x0010, 0x0010, 0x0010, 0x0010, 
	0x0010, 0x0010, 0x0010, 0x0010, 
	0x0010, 0x0010, 0x0010, 0x0010, 
	0x0010, 0x0010, 0x0010, 0x0010, 
	0x0010, 0x0010, 0x0010, 0x0010, 
	0x0010, 0x0010, 0x0010, 0x0010, 
	0x0010, 0x0010, 0x0010, 0x0010, 
	0x0010, 0x0010, 0x0010, 0x0010, 
	0x0010, 0x0010, 0x0010, 0x0010, 
	0x0010, 0x0010, 0x0010, 0x0010, 
	0x0010, 0x0010, 0x0010, 0x0010, 
	0x0010, 0x0010, 0x0010, 0x0010, 
	0x0010, 0x0010, 0x0010, 0x0010, 
	0x0010, 0x0010, 0x0010, 0x0010, 
	0x0010, 0x0010, 0x0010, 0x0010, 
	0x0010, 0x0010, 0x0010, 0x0010, 
	0x0010, 0x0010, 0x0010, 0x0010, 
	0x0010, 0x0010, 0x0010, 0x0010, 
};

lwmovie::vlc::lwmVlcValue8 lwmovie::vlc::mb_addr_inc[2048];

/* Macro for filling up the decoding table for mb_addr_inc */
lwmUInt8 ASSIGN1(int start, int end, int step, lwmUInt8 val, lwmUInt8 num)
{
	for (int i = start; i < end; i+= step)
	{
		for (int j = 0; j < step; j++)
		{
			lwmovie::vlc::mb_addr_inc[i+j].value = val;
			lwmovie::vlc::mb_addr_inc[i+j].num_bits = num;
		}
		val--;
	}
	return val;
}

static void init_mb_addr_inc()
{
	for (int i = 0; i < 8; i++)
	{
		lwmovie::vlc::mb_addr_inc[i].value = lwmovie::vlc::UERROR8;
		lwmovie::vlc::mb_addr_inc[i].num_bits = 0;
	}

	lwmovie::vlc::mb_addr_inc[8].value = lwmovie::vlc::MACRO_BLOCK_ESCAPE;
	lwmovie::vlc::mb_addr_inc[8].num_bits = 11;

	for (int i = 9; i < 15; i++)
	{
		lwmovie::vlc::mb_addr_inc[i].value = lwmovie::vlc::UERROR8;
		lwmovie::vlc::mb_addr_inc[i].num_bits = 0;
	}

	lwmovie::vlc::mb_addr_inc[15].value = lwmovie::vlc::MACRO_BLOCK_STUFFING;
	lwmovie::vlc::mb_addr_inc[15].num_bits = 11;

	for (int i = 16; i < 24; i++)
	{
		lwmovie::vlc::mb_addr_inc[i].value = lwmovie::vlc::UERROR8;
		lwmovie::vlc::mb_addr_inc[i].num_bits = 0;
	}

	lwmUInt8 val = 33;

	val = ASSIGN1(24, 36, 1, val, 11);
	val = ASSIGN1(36, 48, 2, val, 10);
	val = ASSIGN1(48, 96, 8, val, 8);
	val = ASSIGN1(96, 128, 16, val, 7);
	val = ASSIGN1(128, 256, 64, val, 5);
	val = ASSIGN1(256, 512, 128, val, 4);
	val = ASSIGN1(512, 1024, 256, val, 3);
	val = ASSIGN1(1024, 2048, 1024, val, 1);
}


lwmovie::vlc::lwmVlcValue8 lwmovie::vlc::mb_type_P[64];
lwmovie::vlc::lwmVlcValue8 lwmovie::vlc::mb_type_B[64];

static void ASSIGN2(int start, int end, bool quant, bool motion_forward, bool motion_backward, bool pattern, bool intra, lwmUInt8 num, lwmovie::vlc::lwmVlcValue8 *mb_type)
{
	for (int i = start; i < end; i ++)
	{
		lwmUInt8 flags = 0;
		if(quant) flags |= lwmovie::vlc::MB_FLAG_QUANT;
		if(motion_forward) flags |= lwmovie::vlc::MB_FLAG_MOTION_FORWARD;
		if(motion_backward) flags |= lwmovie::vlc::MB_FLAG_MOTION_BACKWARD;
		if(pattern) flags |= lwmovie::vlc::MB_FLAG_PATTERN;
		if(intra) flags |= lwmovie::vlc::MB_FLAG_INTRA;

		mb_type[i].value = flags;
		mb_type[i].num_bits = num;
	}
}

static void init_mb_type_P()
{
	lwmovie::vlc::mb_type_P[0].value = 0;
	lwmovie::vlc::mb_type_P[0].num_bits = 0;

	ASSIGN2(1, 2, 1, 0, 0, 0, 1, 6, lwmovie::vlc::mb_type_P);
	ASSIGN2(2, 4, 1, 0, 0, 1, 0, 5, lwmovie::vlc::mb_type_P);
	ASSIGN2(4, 6, 1, 1, 0, 1, 0, 5, lwmovie::vlc::mb_type_P);
	ASSIGN2(6, 8, 0, 0, 0, 0, 1, 5, lwmovie::vlc::mb_type_P);
	ASSIGN2(8, 16, 0, 1, 0, 0, 0, 3, lwmovie::vlc::mb_type_P);
	ASSIGN2(16, 32, 0, 0, 0, 1, 0, 2, lwmovie::vlc::mb_type_P);
	ASSIGN2(32, 64, 0, 1, 0, 1, 0, 1, lwmovie::vlc::mb_type_P);
}

static void init_mb_type_B()
{
	lwmovie::vlc::mb_type_B[0].value = 0;
	lwmovie::vlc::mb_type_B[0].num_bits = 0;

	ASSIGN2(1, 2, 1, 0, 0, 0, 1, 6, lwmovie::vlc::mb_type_B);
	ASSIGN2(2, 3, 1, 0, 1, 1, 0, 6, lwmovie::vlc::mb_type_B);
	ASSIGN2(3, 4, 1, 1, 0, 1, 0, 6, lwmovie::vlc::mb_type_B);
	ASSIGN2(4, 6, 1, 1, 1, 1, 0, 5, lwmovie::vlc::mb_type_B);
	ASSIGN2(6, 8, 0, 0, 0, 0, 1, 5, lwmovie::vlc::mb_type_B);
	ASSIGN2(8, 12, 0, 1, 0, 0, 0, 4, lwmovie::vlc::mb_type_B);
	ASSIGN2(12, 16, 0, 1, 0, 1, 0, 4, lwmovie::vlc::mb_type_B);
	ASSIGN2(16, 24, 0, 0, 1, 0, 0, 3, lwmovie::vlc::mb_type_B);
	ASSIGN2(24, 32, 0, 0, 1, 1, 0, 3, lwmovie::vlc::mb_type_B);
	ASSIGN2(32, 48, 0, 1, 1, 0, 0, 2, lwmovie::vlc::mb_type_B);
	ASSIGN2(48, 64, 0, 1, 1, 1, 0, 2, lwmovie::vlc::mb_type_B);
}

lwmovie::vlc::lwmVlcValueS8 lwmovie::vlc::motion_vectors[2048];

static lwmSInt8 ASSIGN3(int start, int end, int step, lwmSInt8 val, int num)
{
	for (int i = start; i < end; i+= step)
	{
		for (int j = 0; j < step / 2; j++)
		{
			lwmovie::vlc::motion_vectors[i+j].value = val;
			lwmovie::vlc::motion_vectors[i+j].num_bits = num;
		}
		for (int j = step / 2; j < step; j++)
		{
			lwmovie::vlc::motion_vectors[i+j].value = -val;
			lwmovie::vlc::motion_vectors[i+j].num_bits = num;
		}
		val--;
	}
	return val;
}

static void init_motion_vectors()
{
	lwmSInt8 val = 16;

	for (int i = 0; i < 24; i++)
	{
		lwmovie::vlc::motion_vectors[i].value = lwmovie::vlc::ERROR8;
		lwmovie::vlc::motion_vectors[i].num_bits = 0;
	}

	val = ASSIGN3(24, 36, 2, val, 11);	// 6
	val = ASSIGN3(36, 48, 4, val, 10);	// 3
	val = ASSIGN3(48, 96, 16, val, 8);	// 3
	val = ASSIGN3(96, 128, 32, val, 7);	// 1
	val = ASSIGN3(128, 256, 128, val, 5);	// 1
	val = ASSIGN3(256, 512, 256, val, 4);	// 1
	val = ASSIGN3(512, 1024, 512, val, 3);	// 1
	val = ASSIGN3(1024, 2048, 1024, val, 1); // 1
}

static void decodeInitTables()
{
	init_mb_addr_inc();
	init_mb_type_P();
	init_mb_type_B();
	init_motion_vectors();

	//MPEGnet.jrevdct.init_pre_idct();
}

namespace lwmovie
{
	class lwmVlcStaticInitializer
	{
	public:
		lwmVlcStaticInitializer()
		{
			decodeInitTables();
		}

		static lwmVlcStaticInitializer instance;
	};

	lwmVlcStaticInitializer lwmVlcStaticInitializer::instance;
}
