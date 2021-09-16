#include <modules/tnm067lab1/utils/scalartocolormapping.h>

namespace inviwo {

void ScalarToColorMapping::clearColors() { baseColors_.clear(); }
void ScalarToColorMapping::addBaseColors(vec4 color) { baseColors_.push_back(color); }

vec4 ScalarToColorMapping::sample(float t) const {
    if (baseColors_.size() == 0) return vec4(t);
    if (baseColors_.size() == 1) return vec4(baseColors_[0]);
    if (t <= 0) return vec4(baseColors_.front());
    if (t >= 1) return vec4(baseColors_.back());
    
    // TODO: use t to select which two base colors to interpolate in-between
    int right = ceil((baseColors_.size() - 1) * t);
    int left = floor((baseColors_.size() - 1) * t);
    
    //    if(right == left) {
    //        right = right + 1;
    //        left = left - 1;
    //    }
    
    // Normalize t
    float min = left / float(baseColors_.size() - 1);
    float max = right / float(baseColors_.size() - 1);
    
    t = (t - min) / (max - min);
    
    // TODO: Interpolate colors in baseColors_ and set dummy color to result
    vec4 finalColor(vec4(baseColors_[left]).r + (vec4(baseColors_[right]).r - vec4(baseColors_[left]).r) * t,
                    vec4(baseColors_[left]).g + (vec4(baseColors_[right]).g - vec4(baseColors_[left]).g) * t,
                    vec4(baseColors_[left]).b + (vec4(baseColors_[right]).b - vec4(baseColors_[left]).b) * t,
                    1);  // dummy color
    
    return finalColor;
}

}  // namespace inviwo
