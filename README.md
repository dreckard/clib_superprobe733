# clib_superprobe733
A library for interacting with a Tracor Northern TN-5500 X-Ray analyzer

This was written in 2011 as part of a software automation retrofit for a JEOL Superprobe 733 EPMA which kept the original PDP-11 based Tracor Northern TN-5500 control computer and communicated with it via serial comms.  The library supports reading the stage position, probe current and spectrometer counts as well as setting the stage position and loading FLEXTRAN scripts into the TN-5500's memory. It also attempts to handle the erratic timing requirements and odd errors that I commonly ran into on my test hardware.
