#!/usr/bin/env python3
## ****************************************************************************
##
##  Licensed to Accellera Systems Initiative Inc. (Accellera) under one or
##  more contributor license agreements.  See the NOTICE file distributed
##  with this work for additional information regarding copyright ownership.
##  Accellera licenses this file to you under the Apache License, Version 2.0
##  (the "License"); you may not use this file except in compliance with the
##  License.  You may obtain a copy of the License at
##
##   http://www.apache.org/licenses/LICENSE-2.0
##
##  Unless required by applicable law or agreed to in writing, software
##  distributed under the License is distributed on an "AS IS" BASIS,
##  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
##  implied.  See the License for the specific language governing
##  permissions and limitations under the License.
##
## ****************************************************************************

"""
Compute expected values for the TLM example.
The DUT computes: v1 = in1 * in2, v2 = v1 * in3
"""

def compute_expected_values():
    cnt = 1
    expected = []

    for i in range(40):
        # Driver sends:
        i1 = cnt
        cnt += 1

        if i & 1:  # odd iteration
            i3 = cnt
            cnt += 1
            i2 = cnt
            cnt += 1
        else:  # even iteration
            i2 = cnt
            cnt += 1
            i3 = cnt
            cnt += 1

        # DUT computes:
        v1 = i1 * i2
        v2 = v1 * i3

        expected.append((v1, v2))
        print(f"i={i:2d}: i1={i1:3d}, i2={i2:3d}, i3={i3:3d} -> v1={v1:5d}, v2={v2:7d}")

    return expected

if __name__ == "__main__":
    expected = compute_expected_values()

    print("\n// C++ format for top.h:")
    print("std::vector<std::pair<unsigned, unsigned>> expected = {")
    for i in range(0, len(expected), 4):
        line = "    "
        for j in range(4):
            if i + j < len(expected):
                v1, v2 = expected[i + j]
                line += f"{{{v1}, {v2}}}"
                if i + j < len(expected) - 1:
                    line += ", "
        print(line.rstrip(", ") + ("," if i + 4 < len(expected) else ""))
    print("};")