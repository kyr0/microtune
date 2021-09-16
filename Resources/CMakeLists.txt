juce_add_binary_data(BinaryData SOURCES
        magic.xml
        image/bg.jpg
        image/microtune_logo.png
        )

# Need to build BinaryData with -fPIC flag on Linux
set_target_properties(BinaryData PROPERTIES
        POSITION_INDEPENDENT_CODE TRUE)