UTF-8,UTF-16 Encoding
---------------------

Encodes UTF-8 to UTF16 little endian of big endian.

    $ ./utf --help
    ./utf [-h|--help] [-v|-vv] -u OUT_ENC | --UTF=OUT_ENC IN_FILE [OUT_FILE]
    
    Option arguments:
    -h, --help  Displays this usage.
    -v, -vv     Toggles the verbosity of the program to level 1 or 2.
    
    Mandatory argument:
    -u OUT_ENC, --UTF=OUT_ENC   Sets the output encoding.
                    Valid values for OUT_ENC: 16LE, 16BE
    
    Positional Arguments:
    IN_FILE     The file to convert.
    [OUT_FILE]  Output file name. If not present, defaults to stdout.