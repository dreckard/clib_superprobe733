# clib_superprobe733
A library for interacting with a Tracor Northern TN-5500 X-Ray analyzer

This was written in 2011 as part of a software automation retrofit for a JEOL Superprobe 733 EPMA which kept the original PDP-11 based Tracor Northern TN-5500 control computer and communicated with it via serial comms.  The library supports reading the stage position, probe current and spectrometer counts as well as setting the stage position and loading FLEXTRAN scripts into the TN-5500's memory. It also attempts to handle the erratic timing requirements and odd errors that I commonly ran into on my test hardware.

#License
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

For the complete terms of the GNU General Public License, please see this URL:
http://www.gnu.org/licenses/gpl-2.0.html
