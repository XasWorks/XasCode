idf_component_register(SRCS "BatteryIndicator.cpp" "DrawBox.cpp" "LittleConsole.cpp"
									"SSD1306.cpp" "SSD1327.cpp" "StringPrimitive.cpp" "ValueBox.cpp"
									"fonts/font-5x8.cpp" "fonts/font-6x8.cpp" "fonts/font-7x9.cpp"
                       INCLUDE_DIRS "include"
                       REQUIRES driver XaI2C)
