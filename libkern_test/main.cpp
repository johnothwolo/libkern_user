//
//  main.m
//  libkern_test
//
//  Created by John Othwolo on 6/1/22.
//  Copyright © 2022 oth. All rights reserved.
//

#include <iostream>
//#include <libkern/c++/OSLib.h>
//#include <libkern/c++/OSArray.h>
//#include <libkern/c++/OSString.h>

#include <fstream>
#include <string>

typedef struct dmode {
    int flags;
    int x;
    int y;
} dmode_t;

int main(int argc, const char * argv[]) {
    // insert code here...
    char *cstr = "Hello, World!";
    
//    OSArray *array = OSArray::withCapacity(10);
//    OSString *str = OSString::withCString(cstr);
//
//    array->setObject(0, str);
    
//    printf("%s\n", str->getCStringNoCopy());

    std::string fname = "/Users/john/Desktop/decomp/vmwaregfx/vmware/Library/Extensions/VMwareGfx.kext/Contents/MacOS/VMwareGfx";
    dmode_t modes[40] = {0};
    std::fstream file(fname);
    file.seekg(0xbb60);
    file.read((char*)&modes[0], sizeof(modes));
    
    for (auto &mode : modes){
        std::cout << "{ "<<mode.flags<<", "<<mode.x<<", "<<mode.y<< " },\n";
    }
        
    
    return 0;
}
