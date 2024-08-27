#ifndef GLOBALS_H
#define GLOBALS_H

#include <iostream>
//this is intended to just be a space to store variables that need to be present across multiple files, but generally not all

namespace Chunks{
  const int size = 16; //16x16
  const int height = 256;
}

namespace Render{
  const int renderDistance = 5;
}

namespace Screen{ // 1600 x 900 px window
  const float width = 1600;
  const float height = 900;
}

#endif
