# Inputs
# - NUM_CHANNELS: Number of channels
# - NUM_STREAMS: Number of streams per channel

macro(generate_ecic FILEPATH DIRECTION NUM_CHANNELS NUM_STREAMS)

set(CONTENT "<?xml version=\"1.0\" encoding=\"UTF-8\"?>

<!--
The MIT Licence

Copyright (c) 2020 Airbus Operations S.A.S

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the \"Software\"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
-->

<ED247ComponentInstanceConfiguration ComponentType=\"Virtual\" Name=\"VirtualComponent_1\" Comment=\"\" StandardRevision=\"A\" Identifier=\"1\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"ED247A_ECIC.xsd\">
	<Channels>")

math(EXPR NUM_CHANNELS "${NUM_CHANNELS}-1")
math(EXPR NUM_STREAMS "${NUM_STREAMS}-1")

foreach(ICHANNEL RANGE ${NUM_CHANNELS})

    math(EXPR PORT "4000+${ICHANNEL}")
    string(APPEND CONTENT "
        <MultiChannel Name=\"C${ICHANNEL}\">
            <FrameFormat StandardRevision=\"A\"/>
            <ComInterface>
                <UDP_Sockets>
                    <UDP_Socket DstIP=\"224.1.1.1\" MulticastInterfaceIP=\"${TEST_MULTICAST_INTERFACE_IP}\" DstPort=\"${PORT}\" Direction=\"${DIRECTION}\"/>
                </UDP_Sockets>
            </ComInterface>
            <Streams>")

    foreach(ISTREAM RANGE ${NUM_STREAMS})

        string(APPEND CONTENT "
                <A429_Stream UID=\"${ISTREAM}\" Name=\"C${ICHANNEL}_S${ISTREAM}\" Direction=\"${DIRECTION}\"/>")

    endforeach(ISTREAM RANGE ${NUM_STREAMS})

    string(APPEND CONTENT "
            </Streams>
        </MultiChannel>")

endforeach(ICHANNEL RANGE ${NUM_CHANNELS})

string(APPEND CONTENT "    
	</Channels>
</ED247ComponentInstanceConfiguration>
")

file(WRITE ${FILEPATH} ${CONTENT})

endmacro()

