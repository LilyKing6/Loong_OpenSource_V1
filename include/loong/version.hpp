// Copyright (c) 2023-2026 Lily King.
#pragma once

#include <string_view>

namespace loong {

// full product name
inline constexpr std::string_view kSoftName = "Loong Programming Language";
// short identifier
inline constexpr std::string_view kCodeName = "Loong";

inline constexpr int kCopyrightYear = 2024;
inline constexpr int kVersionMajor = 0;
inline constexpr int kVersionMinor = 2;
inline constexpr int kVersionBuild = 0;
inline constexpr int kVersionQfe = 0;

// edition/channel identifier
inline constexpr std::string_view kVersionBuildType = "loong_opensource";
inline constexpr long kReleaseDate = 20240901L;

// extract build timestamp from __DATE__ and __TIME__
#define BUILD_YEAR     ((((__DATE__ [7] - '0') * 10 + (__DATE__ [8] - '0')) * 10 \
                                     + (__DATE__ [9] - '0')) * 10 + (__DATE__ [10] - '0'))
#define BUILD_MONTH    (__DATE__ [2] == 'n' ? (__DATE__ [1] == 'a' ? 1 : 6) \
                                 : __DATE__ [2] == 'b' ? 2 \
                                 : __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 3 : 4) \
                                 : __DATE__ [2] == 'y' ? 5 \
                                 : __DATE__ [2] == 'l' ? 7 \
                                 : __DATE__ [2] == 'g' ? 8 \
                                 : __DATE__ [2] == 'p' ? 9 \
                                 : __DATE__ [2] == 't' ? 10 \
                                 : __DATE__ [2] == 'v' ? 11 : 12)
#define BUILD_DAY      ((__DATE__ [4] == ' ' ? 0 : __DATE__ [4] - '0') * 10 \
                                 + (__DATE__ [5] - '0'))
#define BUILD_HOUR     ((__TIME__ [0] - '0') * 10 + (__TIME__ [1] - '0'))
#define BUILD_MINUTE   ((__TIME__ [3] - '0') * 10 + (__TIME__ [4] - '0'))
#define BUILD_SECOND   ((__TIME__ [6] - '0') * 10 + (__TIME__ [7] - '0'))

} // namespace loong
