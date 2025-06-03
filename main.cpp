#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include "Core/Application.h"
#include "Graphics 2D/Image.h"

#include "glm.hpp"
#include "gli.hpp"

//#include <Core/Application.h>

int main() {
  
    Application app;
    app.Init();

    //Image img;
    //img.LoadFromDiskStandard("sprite.png");

    app.Run();
    return 0;
}