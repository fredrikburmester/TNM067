#include <modules/tnm067lab2/processors/marchingtetrahedra.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/util/indexmapper.h>
#include <inviwo/core/util/assertion.h>
#include <inviwo/core/network/networklock.h>
#include <modules/tnm067lab1/utils/interpolationmethods.h>

namespace inviwo {

size_t MarchingTetrahedra::HashFunc::max = 1;

const ProcessorInfo MarchingTetrahedra::processorInfo_{
    "org.inviwo.MarchingTetrahedra",  // Class identifier
    "Marching Tetrahedra",            // Display name
    "TNM067",                         // Category
    CodeState::Stable,                // Code state
    Tags::None,                       // Tags
};
const ProcessorInfo MarchingTetrahedra::getProcessorInfo() const { return processorInfo_; }

MarchingTetrahedra::MarchingTetrahedra()
: Processor()
, volume_("volume")
, mesh_("mesh")
, isoValue_("isoValue", "ISO value", 0.5f, 0.0f, 1.0f) {
    
    addPort(volume_);
    addPort(mesh_);
    
    addProperty(isoValue_);
    
    isoValue_.setSerializationMode(PropertySerializationMode::All);
    
    volume_.onChange([&]() {
        if (!volume_.hasData()) {
            return;
        }
        NetworkLock lock(getNetwork());
        float iso = (isoValue_.get() - isoValue_.getMinValue()) /
        (isoValue_.getMaxValue() - isoValue_.getMinValue());
        const auto vr = volume_.getData()->dataMap_.valueRange;
        isoValue_.setMinValue(static_cast<float>(vr.x));
        isoValue_.setMaxValue(static_cast<float>(vr.y));
        isoValue_.setIncrement(static_cast<float>(glm::abs(vr.y - vr.x) / 50.0));
        isoValue_.set(static_cast<float>(iso * (vr.y - vr.x) + vr.x));
        isoValue_.setCurrentStateAsDefault();
    });
}

void MarchingTetrahedra::process() {
    auto volume = volume_.getData()->getRepresentation<VolumeRAM>();
    MeshHelper mesh(volume_.getData());
    
    const auto& dims = volume->getDimensions();
    MarchingTetrahedra::HashFunc::max = dims.x * dims.y * dims.z;
    
    const float iso = isoValue_.get();
    
    util::IndexMapper3D indexInVolume(dims);
    
    const static size_t tetrahedraIds[6][4] = {{0, 1, 2, 5}, {1, 3, 2, 5}, {3, 2, 5, 7},
        {0, 2, 4, 5}, {6, 4, 2, 5}, {6, 7, 5, 2}};
    
    size3_t pos{};
    for (pos.z = 0; pos.z < dims.z - 1; ++pos.z) {
        for (pos.y = 0; pos.y < dims.y - 1; ++pos.y) {
            for (pos.x = 0; pos.x < dims.x - 1; ++pos.x) {
                // Step 1: create current cell
                
                // The DataPoint index should be the 1D-index for the DataPoint in the cell
                // Use volume->getAsDouble to query values from the volume
                // Spatial position should be between 0 and 1
                Cell c;
                size_t index = 0;
                
                for (size_t z = 0; z < 2; ++z) {
                    for (size_t y = 0; y < 2; ++y) {
                        for (size_t x = 0; x < 2; ++x) {
                            vec3 cellPos(x, y, z);
                            
                            vec3 scaledCellPos = calculateDataPointPos(pos, cellPos, dims);
                            size_t cellIndex = calculateDataPointIndexInCell(cellPos);
                            float value = volume->getAsDouble(vec3{pos.x + x, pos.y + y, pos.z + z});
                            
//                            c.dataPoints[cellIndex] = MarchingTetrahedra::DataPoint{scaledCellPos, value, cellIndex};
                            
                            c.dataPoints[index].pos = scaledCellPos;
                            c.dataPoints[index].value = value;
                            c.dataPoints[index].index = cellIndex;

                            index++;
                        }
                    }
                }
                
                
                
                // Step 2: Subdivide cell into tetrahedra (hint: use tetrahedraIds)
                std::vector<Tetrahedra> tetrahedras;
                
                // Temporary tetrahedra
                Tetrahedra tetra;
                
                // Go through all types of tetrahedra to assign 6 of them per cell
                for (size_t i = 0; i < 6; i++) {
                    // Go though all 4 indexies for each tetrahedra and assign the correct cell index, value and position.
                    for (size_t j = 0; j < 4; j++) {
                        tetra.dataPoints[j] = c.dataPoints[tetrahedraIds[i][j]];
                    }
                    
                    // Save all 6 tetrahedra in the vector (the vector will contain 6 * 8 tetrahedras)
                    tetrahedras.push_back(tetra);
                }
                
                for (const Tetrahedra& tetrahedra : tetrahedras) {
                    // Step three: Calculate for tetra case index
                    int caseId = 0;
                    
                    size_t tI0 = tetrahedra.dataPoints[0].index;
                    size_t tI1 = tetrahedra.dataPoints[1].index;
                    size_t tI2 = tetrahedra.dataPoints[2].index;
                    size_t tI3 = tetrahedra.dataPoints[3].index;
                    
                    

                    float tV0 = tetrahedra.dataPoints[0].value;
                    float tV1 = tetrahedra.dataPoints[1].value;
                    float tV2 = tetrahedra.dataPoints[2].value;
                    float tV3 = tetrahedra.dataPoints[3].value;

                    vec3 tPos0 = tetrahedra.dataPoints[0].pos;
                    vec3 tPos1 = tetrahedra.dataPoints[1].pos;
                    vec3 tPos2 = tetrahedra.dataPoints[2].pos;
                    vec3 tPos3 = tetrahedra.dataPoints[3].pos;
                    
                    
                    size_t i = 0;
                    for(auto& t : tetrahedra.dataPoints) {
                        if (t.value < iso) caseId += pow(2, i);
                        i++;
                    }
                    
                    // step four: Extract triangles
                    
                    switch (caseId) {
                        case 0:
                        case 15:
                            break;
                            
                        case 1:
                        case 14: {
                            vec3 interpolatedValue0 = tPos0 + (tPos1 - tPos0) * (iso - tV0) / (tV1 - tV0);
                            vec3 interpolatedValue1 = tPos0 + (tPos3 - tPos0) * (iso - tV0) / (tV3 - tV0);
                            vec3 interpolatedValue2 = tPos0 + (tPos2 - tPos0) * (iso - tV0) / (tV2 - tV0);
                            
                            auto _tI0 = mesh.addVertex(interpolatedValue0, tI0, tI1);
                            auto _tI1 = mesh.addVertex(interpolatedValue1, tI0, tI3);
                            auto _tI2 = mesh.addVertex(interpolatedValue2, tI0, tI2);
                            
                            if (caseId == 1) {
                                mesh.addTriangle(_tI0, _tI1, _tI2);
                            } else {
                                mesh.addTriangle(_tI0, _tI2, _tI1);
                            }
                            
                            break;
                        }
                            
                        case 2:
                        case 13: {
                            break;
                        }
                            
                        case 3:
                        case 12: {
                            break;
                        }
                            
                        case 4:
                        case 11: {
                            break;
                        }
                            
                        case 5:
                        case 10: {
                            break;
                        }
                            
                        case 6:
                        case 9: {
                            break;
                        }
                            
                        case 7:
                        case 8: {
                            break;
                        }
                    }
                }
            }
        }
    }
    
    mesh_.setData(mesh.toBasicMesh());
}

int MarchingTetrahedra::calculateDataPointIndexInCell(ivec3 index3D) {
    // TODO: TASK 5: map 3D index to 2D
    return 1 * index3D[0] + 2 * index3D[1] + 4 * index3D[2];
}

vec3 MarchingTetrahedra::calculateDataPointPos(size3_t posVolume, ivec3 posCell, ivec3 dims) {
    // TODO: TASK 5: scale DataPoint position with dimensions to be between 0 and 1
    //std::cout << "posVolume: " << posVolume[0] << " " << posVolume[1] << " " << posVolume[2] << "\n\n\n";
    //std::cout << "posCell: " << posCell[0] << " " << posCell[1] << " " << posCell[2] << "\n\n\n";
    //std::cout << "dims: " << dims[0] << " " << dims[1] << " " << dims[2] << "\n\n\n";
    
    // posVolume: the cell position in the volume
    // posCell: the position within the cell
    // dims: the dimention of the volume
    
    // return: the position of the data point within the volume (scaled between 0, 1)
    
    //2^dim antal kuber
    
    float x = ( float(posVolume[0] + posCell[0]) ) / float(dims[0] - 1);
    float y = ( float(posVolume[1] + posCell[1]) ) / float(dims[1] - 1);
    float z = ( float(posVolume[2] + posCell[2]) ) / float(dims[2] - 1);
    
    return vec3(x, y, z);
}

MarchingTetrahedra::MeshHelper::MeshHelper(std::shared_ptr<const Volume> vol)
: edgeToVertex_()
, vertices_()
, mesh_(std::make_shared<BasicMesh>())
, indexBuffer_(mesh_->addIndexBuffer(DrawType::Triangles, ConnectivityType::None)) {
    mesh_->setModelMatrix(vol->getModelMatrix());
    mesh_->setWorldMatrix(vol->getWorldMatrix());
}

void MarchingTetrahedra::MeshHelper::addTriangle(size_t i0, size_t i1, size_t i2) {
    IVW_ASSERT(i0 != i1, "i0 and i1 should not be the same value");
    IVW_ASSERT(i0 != i2, "i0 and i2 should not be the same value");
    IVW_ASSERT(i1 != i2, "i1 and i2 should not be the same value");
    
    indexBuffer_->add(static_cast<glm::uint32_t>(i0));
    indexBuffer_->add(static_cast<glm::uint32_t>(i1));
    indexBuffer_->add(static_cast<glm::uint32_t>(i2));
    
    const auto a = std::get<0>(vertices_[i0]);
    const auto b = std::get<0>(vertices_[i1]);
    const auto c = std::get<0>(vertices_[i2]);
    
    const vec3 n = glm::normalize(glm::cross(b - a, c - a));
    std::get<1>(vertices_[i0]) += n;
    std::get<1>(vertices_[i1]) += n;
    std::get<1>(vertices_[i2]) += n;
}

std::shared_ptr<BasicMesh> MarchingTetrahedra::MeshHelper::toBasicMesh() {
    for (auto& vertex : vertices_) {
        // Normalize the normal of the vertex
        std::get<1>(vertex) = glm::normalize(std::get<1>(vertex));
    }
    mesh_->addVertices(vertices_);
    return mesh_;
}

std::uint32_t MarchingTetrahedra::MeshHelper::addVertex(vec3 pos, size_t i, size_t j) {
    IVW_ASSERT(i != j, "i and j should not be the same value");
    if (j < i) std::swap(i, j);
    
    auto [edgeIt, inserted] = edgeToVertex_.try_emplace(std::make_pair(i, j), vertices_.size());
    if (inserted) {
        vertices_.push_back({pos, vec3(0, 0, 0), pos, vec4(0.7f, 0.7f, 0.7f, 1.0f)});
    }
    return static_cast<std::uint32_t>(edgeIt->second);
}

}  // namespace inviwo
