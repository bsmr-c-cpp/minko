/*
Copyright (c) 2014 Aerys

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "minko/file/WriterOptions.hpp"
#include "minko/math/Vector2.hpp"
#include "minko/render/MipFilter.hpp"

using namespace minko;
using namespace minko::file;
using namespace minko::math;
using namespace minko::render;
using namespace minko::serialize;

WriterOptions::WriterOptions() :
    _embedAll(false),
    _addBoundingBoxes(false),
    _outputAssetUriFunction([=](const std::string& str) -> std::string { return str; }),
    _imageFormat(ImageFormat::PNG),
    _textureFormats(),
    _compressTexture(true),
    _generateMipmaps(true),
    _upscaleTextureWhenProcessedForMipmapping(true),
    _textureMaxResolution(Vector2::create(2048, 2048)),
    _mipFilter(MipFilter::LINEAR),
    _optimizeForNormalMapping(false)
{
}
