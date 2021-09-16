#include <inviwo/core/util/logcentral.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/tnm067lab1/processors/imageupsampler.h>
#include <modules/tnm067lab1/utils/interpolationmethods.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/util/imageramutils.h>

namespace inviwo {

namespace detail {

template <typename T>
void upsample(ImageUpsampler::IntepolationMethod method, const LayerRAMPrecision<T>& inputImage,
              LayerRAMPrecision<T>& outputImage) {
    using F = typename float_type<T>::type;
    
    const size2_t inputSize = inputImage.getDimensions();
    const size2_t outputSize = outputImage.getDimensions();
    
    const T* inPixels = inputImage.getDataTyped();
    T* outPixels = outputImage.getDataTyped();
    
    auto inIndex = [&inputSize](auto pos) -> size_t {
        pos = glm::clamp(pos, decltype(pos)(0), decltype(pos)(inputSize - size2_t(1)));
        return pos.x + pos.y * inputSize.x;
    };
    auto outIndex = [&outputSize](auto pos) -> size_t {
        pos = glm::clamp(pos, decltype(pos)(0), decltype(pos)(outputSize - size2_t(1)));
        return pos.x + pos.y * outputSize.x;
    };
    
    util::forEachPixel(outputImage, [&](ivec2 outImageCoords) {
        // outImageCoords: Exact pixel coordinates in the output image currently writing to
        // inImageCoords: Relative coordinates of outImageCoords in the input image, might be
        // between pixels
        dvec2 inImageCoords =
        ImageUpsampler::convertCoordinate(outImageCoords, inputSize, outputSize);
        
        T finalColor(0);
        
        // DUMMY COLOR, remove or overwrite this bellow
        //        finalColor = inPixels[inIndex(
        //            glm::clamp(size2_t(outImageCoords), size2_t(0), size2_t(inputSize - size2_t(1))))];
        
        switch (method) {
            case ImageUpsampler::IntepolationMethod::PiecewiseConstant: {
                // Task 6
                
                finalColor = inPixels[inIndex(ivec2(inImageCoords))];
                break;
            }
            case ImageUpsampler::IntepolationMethod::Bilinear: {
                // TASK 9: Implement and update finalColor
                
                // round down to the corresponding pixel left corner?
                ivec2 inImageCoords_floor = glm::floor(inImageCoords);
                
                // get the color for each pixel adjecent up to the right.
                std::array<T, 4> values = {
                    inPixels[inIndex(inImageCoords_floor)],
                    inPixels[inIndex(inImageCoords_floor + ivec2(1, 0))],
                    inPixels[inIndex(inImageCoords_floor + ivec2(0, 1))],
                    inPixels[inIndex(inImageCoords_floor + ivec2(1, 1))]
                };
                
                // keep only the decimal so we get a number between 0 and 1
                double x = inImageCoords.x - floor(inImageCoords.x);
                double y = inImageCoords.y - floor(inImageCoords.y);
                
                finalColor = TNM067::Interpolation::bilinear(values, x, y);
                
                break;
            }
            case ImageUpsampler::IntepolationMethod::Biquadratic: {
                // Move to center of pixels
                inImageCoords -= 0.5;
                ivec2 samplePos(glm::floor(inImageCoords));
                
                // InPixels handles clamping of values
                std::array<T, 9> values = {
                    inPixels[inIndex(samplePos)],
                    inPixels[inIndex(samplePos + ivec2(1, 0))],
                    inPixels[inIndex(samplePos + ivec2(2, 0))],
                    inPixels[inIndex(samplePos + ivec2(0, 1))],
                    inPixels[inIndex(samplePos + ivec2(1, 1))],
                    inPixels[inIndex(samplePos + ivec2(2, 1))],
                    inPixels[inIndex(samplePos + ivec2(0, 2))],
                    inPixels[inIndex(samplePos + ivec2(1, 2))],
                    inPixels[inIndex(samplePos + ivec2(2, 2))],
                };

                double x = (inImageCoords.x - int(inImageCoords.x)) / 2.0;
                double y = (inImageCoords.y - int(inImageCoords.y)) / 2.0;
                finalColor = TNM067::Interpolation::biQuadratic(values, x, y);
                
                break;
            }
            case ImageUpsampler::IntepolationMethod::Barycentric: {
                // TASK 11: Implement and update finalColor
                // round down to the corresponding pixel left corner?
                ivec2 inImageCoords_floor = glm::floor(inImageCoords);
                
                // get the color for each pixel adjecent up to the right.
                std::array<T, 4> values = {
                    inPixels[inIndex(inImageCoords_floor)],
                    inPixels[inIndex(inImageCoords_floor + ivec2(1, 0))],
                    inPixels[inIndex(inImageCoords_floor + ivec2(0, 1))],
                    inPixels[inIndex(inImageCoords_floor + ivec2(1, 1))]
                };
                
                // keep only the decimal so we get a number between 0 and 1
                double x = inImageCoords.x - floor(inImageCoords.x);
                double y = inImageCoords.y - floor(inImageCoords.y);
                
                finalColor = TNM067::Interpolation::barycentric(values, x, y);
                break;
            }
            default:
                break;
        }
        
        outPixels[outIndex(outImageCoords)] = finalColor;
    });
}

}  // namespace detail

const ProcessorInfo ImageUpsampler::processorInfo_{
    "org.inviwo.imageupsampler",  // Class identifier
    "Image Upsampler",            // Display name
    "TNM067",                     // Category
    CodeState::Experimental,      // Code state
    Tags::None,                   // Tags
};
const ProcessorInfo ImageUpsampler::getProcessorInfo() const { return processorInfo_; }

ImageUpsampler::ImageUpsampler()
: Processor()
, inport_("inport", true)
, outport_("outport", true)
, interpolationMethod_("interpolationMethod", "Interpolation Method",
                       {
    {"piecewiseconstant", "Piecewise Constant (Nearest Neighbor)",
        IntepolationMethod::PiecewiseConstant},
    {"bilinear", "Bilinear", IntepolationMethod::Bilinear},
    {"biquadratic", "Biquadratic", IntepolationMethod::Biquadratic},
    {"barycentric", "Barycentric", IntepolationMethod::Barycentric},
}) {
    addPort(inport_);
    addPort(outport_);
    addProperty(interpolationMethod_);
}

void ImageUpsampler::process() {
    auto inputImage = inport_.getData();
    if (inputImage->getDataFormat()->getComponents() != 1) {
        LogError("The ImageUpsampler processor does only support single channel images");
    }
    
    auto inSize = inport_.getData()->getDimensions();
    auto outDim = outport_.getDimensions();
    
    auto outputImage = std::make_shared<Image>(outDim, inputImage->getDataFormat());
    outputImage->getColorLayer()->setSwizzleMask(inputImage->getColorLayer()->getSwizzleMask());
    outputImage->getColorLayer()
    ->getEditableRepresentation<LayerRAM>()
    ->dispatch<void, dispatching::filter::Scalars>([&](auto outRep) {
        auto inRep = inputImage->getColorLayer()->getRepresentation<LayerRAM>();
        detail::upsample(interpolationMethod_.get(), *(const decltype(outRep))(inRep), *outRep);
    });
    
    outport_.setData(outputImage);
}

dvec2 ImageUpsampler::convertCoordinate(ivec2 outImageCoords, size2_t inputSize, size2_t outputSize) {
    // TODO implement
    dvec2 c(outImageCoords);
    
    // TASK 5: Convert the outImageCoords to its coordinates in the input image
    c = c * dvec2(inputSize) / dvec2(outputSize);
    
    return c;
}

}  // namespace inviwo
