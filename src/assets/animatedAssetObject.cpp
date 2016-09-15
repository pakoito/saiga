#include "saiga/assets/animatedAssetObject.h"

#include "saiga/assets/asset.h"
#include "saiga/camera/camera.h"

#include "saiga/animation/boneShader.h"

void AnimatedAssetObject::setAnimation(int id)
{
    assert(id>=0 && id < (int)asset->animations.size());
    activeAnimation = id;
    animationTotalTime = asset->animations[id].duration;
}

void AnimatedAssetObject::init(AnimatedAsset *_asset)
{
    assert(_asset);
    this->asset = _asset;
    BoneShader* bs = static_cast<BoneShader*>(asset->shader);
    boneMatricesBuffer.init(bs,bs->location_boneMatricesBlock);
    setAnimation(0);
}

void AnimatedAssetObject::updateAnimation(float dt)
{
    animationTimeAtUpdate += dt ;
    //loop animation constantly
    if(animationTimeAtUpdate >= animationTotalTime)
        animationTimeAtUpdate -= animationTotalTime;
}

void AnimatedAssetObject::interpolateAnimation(float dt, float alpha)
{
    animationTimeAtRender = animationTimeAtUpdate + dt * alpha;
    if(animationTimeAtRender >= animationTotalTime)
        animationTimeAtRender -= animationTotalTime;
    asset->animations[activeAnimation].getFrame(animationTimeAtRender,currentFrame);
    boneMatricesBuffer.updateBuffer(currentFrame.getBoneMatrices().data(),currentFrame.getBoneMatrices().size()*sizeof(mat4),0);
}


void AnimatedAssetObject::render(Camera *cam)
{
    asset->render(cam,model,boneMatricesBuffer);
}

void AnimatedAssetObject::renderDepth(Camera *cam)
{
    asset->renderDepth(cam,model,boneMatricesBuffer);
}

void AnimatedAssetObject::renderWireframe(Camera *cam)
{
    asset->renderWireframe(cam,model);
}

void AnimatedAssetObject::renderRaw()
{
    asset->renderRaw();
}
