#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdint> // For uint32_t
#include <glm/glm.hpp>

class Belt {
public:
    glm::vec3 pos1;
    glm::vec3 pos2;
    float belt_width = 40.0f;

    Belt(glm::vec3 p1, glm::vec3 p2) : pos1(p1), pos2(p2) {}

    /**
     * Generates an evenly spaced 3D path and records distances for UV mapping.
     */
    void generatePath(float step_size = 5.0f, glm::vec3 dir0 = {0, 0, 1}, glm::vec3 dir1 = {0, 0, 1}, glm::vec3 up = {0, 1, 0}) {
        path.clear();
        laterals.clear();
        normals.clear(); // Ensure the new normals vector is cleared
        path_distances.clear();
        current_up = glm::normalize(up);

        glm::vec3 p0 = pos1;
        glm::vec3 p3 = pos2;

        float dist = glm::distance(p0, p3);
        float dot_dirs = glm::dot(dir0, dir1);

        float base_influence = dist * 0.35f;
        float turn_factor = 1.0f + (1.0f - dot_dirs) * 0.5f;
        float influence = std::max(base_influence * turn_factor, belt_width * 2.0f);

        glm::vec3 p1_ctrl = p0 + (dir0 * influence);
        glm::vec3 p2_ctrl = p3 - (dir1 * influence);

        auto get_bezier_point = [&](float t) {
            float u = 1.0f - t;
            return (u * u * u * p0) + (3.0f * u * u * t * p1_ctrl) + 
                   (3.0f * u * t * t * p2_ctrl) + (t * t * t * p3);
        };

        auto get_bezier_derivative = [&](float t) {
            float u = 1.0f - t;
            return (3.0f * u * u * (p1_ctrl - p0)) + 
                   (6.0f * u * t * (p2_ctrl - p1_ctrl)) + 
                   (3.0f * t * t * (p3 - p2_ctrl));
        };

        // --- DENSE SAMPLING ---
        float est_length = glm::distance(p0, p1_ctrl) + glm::distance(p1_ctrl, p2_ctrl) + glm::distance(p2_ctrl, p3);
        int samples = std::max(20, static_cast<int>(est_length / (step_size * 0.5f)));

        std::vector<float> sample_t;
        std::vector<glm::vec3> sample_pts;
        std::vector<float> distances = {0.0f};

        for (int i = 0; i <= samples; ++i) {
            float t = (float)i / samples;
            sample_t.push_back(t);
            sample_pts.push_back(get_bezier_point(t));
            if (i > 0) {
                distances.push_back(distances.back() + glm::distance(sample_pts[i - 1], sample_pts[i]));
            }
        }

        float total_length = distances.back();

        // --- EVEN RESAMPLING ---
        int num_steps = std::max(1, (int)std::round(total_length / step_size));
        float adjusted_step = total_length / num_steps;

        float target_d = 0.0f;
        size_t sample_idx = 0;

        for (int s = 0; s < num_steps; ++s) {
            while (sample_idx < distances.size() - 1 && distances[sample_idx + 1] < target_d) {
                sample_idx++;
            }

            float d0 = distances[sample_idx];
            float d1 = distances[sample_idx + 1];
            float t_seg = (target_d - d0) / (d1 - d0 + 1e-8f);

            float t_interp = sample_t[sample_idx] + (sample_t[sample_idx + 1] - sample_t[sample_idx]) * t_seg;

            glm::vec3 pt = get_bezier_point(t_interp);
            glm::vec3 tangent = get_bezier_derivative(t_interp);

            if (glm::length(tangent) > 0) tangent = glm::normalize(tangent);
            else tangent = glm::vec3(0, 0, 1); // Fallback
            
            // 1. Calculate the Lateral (Right) vector
            glm::vec3 right_vec = glm::cross(tangent, current_up);
            if (glm::length(right_vec) > 0) {
                right_vec = glm::normalize(right_vec);
            } else {
                right_vec = glm::vec3(1, 0, 0); // Fallback if going straight up
            }
            
            // 2. Calculate the True Surface Normal
            glm::vec3 true_normal = glm::cross(right_vec, tangent);
            if (glm::length(true_normal) > 0) {
                true_normal = glm::normalize(true_normal);
            }
            
            path.push_back(pt);
            laterals.push_back(right_vec);
            normals.push_back(true_normal); // Store the computed normal
            path_distances.push_back(target_d);

            target_d += adjusted_step;
        }

        // --- ADD FINAL POINT ---
        path.push_back(p3);
        path_distances.push_back(total_length);
        
        glm::vec3 final_tangent = get_bezier_derivative(1.0f);
        if (glm::length(final_tangent) > 0) final_tangent = glm::normalize(final_tangent);
        else final_tangent = glm::vec3(0, 0, 1);
        
        glm::vec3 final_right = glm::cross(final_tangent, current_up);
        if (glm::length(final_right) > 0) final_right = glm::normalize(final_right);
        else final_right = glm::vec3(1, 0, 0);

        glm::vec3 final_normal = glm::cross(final_right, final_tangent);
        if (glm::length(final_normal) > 0) final_normal = glm::normalize(final_normal);
        
        laterals.push_back(final_right);
        normals.push_back(final_normal);
    }

    /**
     * Generates indexed vertex data (x,y,z, nx,ny,nz, u,v) and the index buffer.
     */
    void generateGeometry(float uv_scale = 1.0f) {
        vertexData.clear();
        indices.clear();
        if (path.size() < 2) return;

        float half_width = belt_width / 2.0f;
        
        auto push_vertex = [&](glm::vec3 pos, glm::vec3 norm, float u, float v) {
            vertexData.push_back(pos.x);
            vertexData.push_back(pos.y);
            vertexData.push_back(pos.z);
            vertexData.push_back(norm.x);
            vertexData.push_back(norm.y);
            vertexData.push_back(norm.z);
            vertexData.push_back(u);
            vertexData.push_back(v);
        };

        // --- 1. GENERATE UNIQUE VERTICES ---
        for (size_t i = 0; i < path.size(); ++i) {
            glm::vec3 p = path[i];
            glm::vec3 lat = laterals[i];
            glm::vec3 norm = normals[i]; // Fetch the calculated normal!
            
            glm::vec3 L = p - lat * half_width;
            glm::vec3 R = p + lat * half_width;

            float v = (path_distances[i] / belt_width) * uv_scale;

            // Push left and right vertices using the true surface normal
            push_vertex(L, norm, 0.0f, v);
            push_vertex(R, norm, 1.0f, v);
        }

        // --- 2. GENERATE INDICES ---
        for (size_t i = 0; i < path.size() - 1; ++i) {
            uint32_t L0 = i * 2;       // Current Left
            uint32_t R0 = i * 2 + 1;   // Current Right
            uint32_t L1 = (i + 1) * 2; // Next Left
            uint32_t R1 = L1 + 1;      // Next Right

            // Triangle 1 (Counter-Clockwise winding)
            indices.push_back(L0);
            indices.push_back(R0);
            indices.push_back(L1);

            // Triangle 2 (Counter-Clockwise winding)
            indices.push_back(L1);
            indices.push_back(R0);
            indices.push_back(R1);
        }
    }

    void generateGeometryFromTemplateSlice(std::vector<float> SliceTemplate, float TemplateScale = 1.0f, float uv_scale = 1.0f) {
        if (SliceTemplate.empty() || SliceTemplate.size() < 9) return;
        
        int numSliceVerts = SliceTemplate.size() / 3;
        
        // --- NEW: 0. Pre-calculate 2D vertex normals for the slice ---
        std::vector<glm::vec3> sliceNormals(numSliceVerts);
        for (int i = 0; i < numSliceVerts; i++) {
            // Handle wrap-around so the start and end of the loop connect seamlessly
            // (-2 because the very last vertex is a duplicate of the first one)
            int prevIdx = (i == 0) ? (numSliceVerts - 2) : (i - 1); 
            int nextIdx = (i == numSliceVerts - 1) ? 1 : (i + 1);

            glm::vec2 prev(SliceTemplate[prevIdx * 3], SliceTemplate[prevIdx * 3 + 1]);
            glm::vec2 curr(SliceTemplate[i * 3],       SliceTemplate[i * 3 + 1]);
            glm::vec2 next(SliceTemplate[nextIdx * 3], SliceTemplate[nextIdx * 3 + 1]);

            // Get the directions of the two connected edges
            glm::vec2 edge1 = glm::normalize(curr - prev);
            glm::vec2 edge2 = glm::normalize(next - curr);

            // For a Counter-Clockwise shape, the outward normal of vector (x, y) is (y, -x)
            glm::vec2 norm1(edge1.y, -edge1.x); 
            glm::vec2 norm2(edge2.y, -edge2.x);

            // Average the two edge normals to get a smooth vertex normal
            glm::vec2 avgNorm = norm1 + norm2;
            if (glm::length(avgNorm) > 0.0001f) {
                avgNorm = glm::normalize(avgNorm);
            } else {
                avgNorm = norm1; // Fallback for sharp 180-degree folds
            }
            
            // Store as a 3D vector (Z is 0 because it's flat on the template)
            sliceNormals[i] = glm::vec3(avgNorm.x, avgNorm.y, 0.0f);
        }
        // -------------------------------------------------------------

        auto push_vertex = [&](glm::vec3 pos, glm::vec3 norm, float u, float v) {
            vertexData.push_back(pos.x);
            vertexData.push_back(pos.y);
            vertexData.push_back(pos.z);
            vertexData.push_back(norm.x);
            vertexData.push_back(norm.y);
            vertexData.push_back(norm.z);
            vertexData.push_back(u);
            vertexData.push_back(v);
        };

        // 1. Generate Vertices and UVs
        for (size_t i = 0; i < path.size(); ++i) {
            glm::vec3 p = path[i];
            glm::vec3 lat = laterals[i];
            glm::vec3 pathNorm = normals[i];

            glm::vec3 tangent;
            if (i < path.size() - 1) {
                tangent = glm::normalize(path[i + 1] - path[i]);
            } else {
                tangent = glm::normalize(path[i] - path[i - 1]);
            }
            
            glm::mat3 basis = glm::mat3(
                lat,      // X
                pathNorm, // Y
                tangent   // Z
            );

            float v = static_cast<float>(i) / (path.size() - 1);

            for (int vIdx = 0; vIdx < numSliceVerts; vIdx++) {
                glm::vec3 localPos = glm::vec3(
                    SliceTemplate[vIdx * 3],
                    SliceTemplate[vIdx * 3 + 1],
                    SliceTemplate[vIdx * 3 + 2]
                );
                localPos *= TemplateScale;

                // --- NEW: Rotate the 2D normal into 3D space using the basis matrix ---
                glm::vec3 localNorm = sliceNormals[vIdx];
                glm::vec3 worldNorm = glm::normalize(basis * localNorm);

                float u = static_cast<float>(vIdx) / (numSliceVerts - 1);
                glm::vec3 worldPos = p + basis * localPos;
                
                // Push the vertex with its TRUE calculated world normal
                push_vertex(worldPos, worldNorm, u * uv_scale, v * uv_scale);
            }
        }

        // 2. Generate Indices
        int totalVertices = vertexData.size() / 8; 
        int maxQuadIndex = totalVertices - numSliceVerts; 
        uint32_t vertexPointer = 0;

        while (vertexPointer < maxQuadIndex) {
            if ((vertexPointer + 1) % numSliceVerts == 0) {
                vertexPointer++;
                continue;
            }

            indices.push_back(vertexPointer);
            indices.push_back(vertexPointer + numSliceVerts);
            indices.push_back(vertexPointer + 1);

            indices.push_back(vertexPointer + 1);
            indices.push_back(vertexPointer + numSliceVerts);
            indices.push_back(vertexPointer + numSliceVerts + 1);

            vertexPointer += 1; 
        }
    }

    const std::vector<float>& getVertexData() const { return vertexData; }
    size_t getVertexCount() const { return vertexData.size() / 8; }

    const std::vector<uint32_t>& getIndices() const { return indices; }
    size_t getIndexCount() const { return indices.size(); }

    std::vector<float> getSliceTemplate() {
        return SliceTemplate;
    }
private:
    std::vector<glm::vec3> path;
    std::vector<glm::vec3> laterals;
    std::vector<float> path_distances;
    glm::vec3 current_up = {0, 1, 0};
    
    std::vector<float> vertexData;   // Layout: x,y,z, nx,ny,nz, u,v
    std::vector<glm::vec3> normals;
    std::vector<uint32_t> indices;   // Connects the dots

    std::vector<float> SliceTemplate = {
        // x (Width), y (Height), z (Forward - keep 0) 
        -1.0f,   0.0f, 0.0f,   // v4 (Start)
         1.0f,   0.0f, 0.0f,   // v3
         1.0f,   0.9f, 0.0f,   // v2
         0.875f, 1.0f, 0.0f,   // v11
         0.75f,  0.9f, 0.0f,   // v9
         0.7f,   0.6f, 0.0f,   // v6
         0.0f,   0.6f, 0.0f,   // v5
        -0.7f,   0.6f, 0.0f,   // v7
        -0.75f,  0.9f, 0.0f,   // v8
        -0.875f, 1.0f, 0.0f,   // v10
        -1.0f,   0.9f, 0.0f,   // v1
        -1.0f,   0.0f, 0.0f    // v4 (Duplicate to close the loop)
    };
};