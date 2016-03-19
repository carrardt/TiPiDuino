
#====================================================================#
# ASK signal pulse length histogram
#====================================================================#
generate_arduino_firmware(
	RF_analyser
    SKETCH RF_analyser
    PROGRAMMER usbtinyisp
    NO_AUTOLIBS)

# Alternative: by variables
#set(blink_bundled_SKETCH Blink)
#set(blink_bundled_PROGRAMMER usbtinyisp)
#set(blink_bundled_NO_AUTOLIBS true)
#
#generate_arduino_firmware(blink_bundled)
