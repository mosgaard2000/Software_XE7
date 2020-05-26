This tool is used to convert a .dat and .bin file to a .hex file which can be used by the bootloader.

To use the tool to the following:

1. Install nrfutil version 6.0.0a1 (haven't been tested with other versions) which is also included in this repository.

2. Use nrfutil to generate a zip-file.
It takes the compiled hex file and the private key file as inputs and outputs a zip file.

'nrfutil pkg generate --application program_file.hex --hw-version 52 --sd-req 0xB6 --application-version 2 --key-file private.pem C:\\Work\\Output\\Output_Package.zip'

3. Extract the .dat and .bin file from the zip archieve.

4. Start the 'FW_File_Converter.exe' and press 'Open .dat' button.

5. Locate the .dat file (the bin file must be in the same folder).

6. Press 'Convert' to generate "Bootloader_out.hex".

7. "Bootloader_out.hex" can be read by the bootloader.