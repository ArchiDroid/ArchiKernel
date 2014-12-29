#!/system/bin/sh

#     _             _     _ _  __                    _
#    / \   _ __ ___| |__ (_) |/ /___ _ __ _ __   ___| |
#   / _ \ | '__/ __| '_ \| | ' // _ \ '__| '_ \ / _ \ |
#  / ___ \| | | (__| | | | | . \  __/ |  | | | |  __/ |
# /_/   \_\_|  \___|_| |_|_|_|\_\___|_|  |_| |_|\___|_|
#
# Copyright 2014 Łukasz "JustArchi" Domeradzki
# Contact: JustArchi@JustArchi.net
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

echo "
{
	name:I/O,
	elements:[
		{
			SPane:{
				title:"I/O schedulers",
				description:"Set the active I/O elevator algorithm. The scheduler decides how to handle I/O requests and how to handle them."
			}
		},
		{
			SOptionList:{
				title:"Internal storage scheduler",
				default:$("$AKACTION/bracket-option" "/sys/block/mmcblk0/queue/scheduler"),
				action:"bracket-option /sys/block/mmcblk0/queue/scheduler",
				values:[
					$(
						for IOSCHED in $(cat /sys/block/mmcblk0/queue/scheduler | sed -e 's/\[//;s/\]//'); do
							echo "\"$IOSCHED\","
						done
					)
				]
			}
		},
		{
			SOptionList:{
				title:"SD card scheduler",
				default:$("$AKACTION/bracket-option" "/sys/block/mmcblk1/queue/scheduler"),
				action:"bracket-option /sys/block/mmcblk1/queue/scheduler",
				values:[
					$(
						for IOSCHED in $(cat /sys/block/mmcblk1/queue/scheduler | sed -e 's/\[//;s/\]//'); do
							echo "\"$IOSCHED\","
						done
					)
				]
			}
		},
		{
			SSeekBar:{
				title:"Internal storage read-ahead",
				description:"The read-ahead value on the internal phone memory.",
				max:2048, min:128, unit:"kB", step:128,
				default:$("$AKACTION/generic" /sys/block/mmcblk0/queue/read_ahead_kb),
				action:"generic /sys/block/mmcblk0/queue/read_ahead_kb"
			}
		},
		{
			SSeekBar:{
				title:"SD card read-ahead",
				description:"The read-ahead value on the external SD card.",
				max:2048, min:128, unit:"kB", step:128,
				default:$("$AKACTION/generic" /sys/block/mmcblk1/queue/read_ahead_kb),
				action:"generic /sys/block/mmcblk1/queue/read_ahead_kb"
			}
		},
    ]
}
"
