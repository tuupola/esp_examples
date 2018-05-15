/*
 * Copyright (c) 2017 Mika Tuupola
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef BSWAP_16
#define BSWAP_16(x) ((((uint16_t) (x) << 8) & 0xff00) | (((uint16_t) (x) >> 8) & 0xff))
#endif

#ifndef BSWAP_32
#define BSWAP_32(x) ((((uint32_t) (x) << 24) & 0xff000000) | \
                    (((uint32_t) (x) << 8) & 0xff0000) | \
                    (((uint32_t) (x) >> 8) & 0xff00) | \
                    (((uint32_t) (x) >> 24) & 0xff))
#endif