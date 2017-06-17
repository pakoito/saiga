#include "saiga/util/crash.h"


#include "saiga/cuda/cudaHelper.h"
#include "saiga/opengl/opengl.h"
#include "saiga/cuda/cusparseHelper.h"
#include "saiga/cuda/tests/test.h"
#include "saiga/tests/test.h"



#include "saiga/geometry/clipping.h"

#include "saiga/image/freeimage.h"
#include <FreeImagePlus.h>
//#include "saiga/opengl/texture/textureLoader.h"

int main(int argc, char *argv[]) {

    catchSegFaults();


    cout << "asdf" << endl;

    std::vector<std::string> images = {
//        "A002C015_130612_R4MX.848124.tif",
//        "8D0A5523.CR2",
//        "8D0A5579.CR2",
//        "8D0A1390.CR2",
//        "8D0A5553.CR2",
//        "8D0A5554.CR2",
        "8D0A5553.jpg",
        "8D0A5554.jpg",
//        "box.png",
    };

    for(auto str : images){

        Image img;
        ImageMetadata metaData;
        auto ret = FIP::load("textures/" + str,img,&metaData);
        SAIGA_ASSERT(ret);
//        img.flipRB();
        img.to8bitImage();
        ret = FIP::save("textures/" + str + ".jpg",img);
        SAIGA_ASSERT(ret);

        cout << metaData << endl;
//        fipImage fimg;
//        FIP::loadFIP("textures/" + str,fimg);
//        FIP::printAllMetaData(fimg);
    }

}