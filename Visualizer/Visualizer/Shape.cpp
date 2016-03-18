#include "Shape.h"
#include <iostream>

#define EIGEN_DONT_ALIGN_STATICALLY
#include <Eigen/Dense>

#include "GLSL.h"
#include "Program.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

using namespace std;

Shape::Shape() :
    eleBufID(0),
    posBufID(0),
    norBufID(0),
    texBufID(0), 
    vaoID(0)
{
}

Shape::~Shape()
{
}

void Shape::loadMesh(const string &meshName)
{
    // Load geometry
    // Some obj files contain material information.
    // We'll ignore them for this assignment.
    vector<tinyobj::shape_t> shapes;
    vector<tinyobj::material_t> objMaterials;
    string errStr;
    bool rc = tinyobj::LoadObj(shapes, objMaterials, errStr, meshName.c_str());
    if(!rc) {
        cerr << errStr << endl;
    } else {
        posBuf = shapes[0].mesh.positions;
        //norBuf = shapes[0].mesh.normals;
        texBuf = shapes[0].mesh.texcoords;
        eleBuf = shapes[0].mesh.indices;
        
        //initialize all normals to zero to start
        for (size_t v = 0; v < posBuf.size(); v++) {
            norBuf.push_back(0);
        }
        //helper function to actually compute the normals

        ComputeNormals();
    }
}

void Shape::ComputeNormals()
{
    for (int index = 0; index < eleBuf.size(); index += 3)
    {
        // the index of the first vertex of the first face
        uint vertID1 = eleBuf[index];
        uint vertID2 = eleBuf[index + 1];
        uint vertID3 = eleBuf[index + 2];
        
        // the first vertex of the first face
        float vert0[3] = {posBuf[vertID1 * 3], 
                          posBuf[vertID1 * 3 + 1], 
                          posBuf[vertID1 * 3 + 2]}; 
        float vert1[3] = {posBuf[vertID2 * 3], 
                          posBuf[vertID2 * 3 + 1], 
                          posBuf[vertID2 * 3 + 2]}; 
        float vert2[3] = {posBuf[vertID3 * 3], 
                          posBuf[vertID3 * 3 + 1], 
                          posBuf[vertID3 * 3 + 2]}; 

        // more assignments and cross product stuff here
        Eigen::Vector3f edge1(vert1[0] - vert0[0], 
                       vert1[1] - vert0[1], 
                       vert1[2] - vert0[2]);
                       
        Eigen::Vector3f edge2(vert2[0] - vert0[0], 
                       vert2[1] - vert0[1], 
                       vert2[2] - vert0[2]);
                       
        Eigen::Vector3f calculatedNormal = edge1.cross(edge2);

        norBuf[vertID1 * 3] += calculatedNormal.x();
        norBuf[vertID1 * 3 + 1] += calculatedNormal.y();
        norBuf[vertID1 * 3 + 2] += calculatedNormal.z();
        norBuf[vertID2 * 3] += calculatedNormal.x();
        norBuf[vertID2 * 3 + 1] += calculatedNormal.y();
        norBuf[vertID2 * 3 + 2] += calculatedNormal.z();
        norBuf[vertID3 * 3] += calculatedNormal.x();
        norBuf[vertID3 * 3 + 1] += calculatedNormal.y();
        norBuf[vertID3 * 3 + 2] += calculatedNormal.z();
    }

    //normalize each triple in norBuf
    for (int index = 0; index < norBuf.size(); index += 3)
    {
        // the index of the first vertex of the first face
        Eigen::Vector3f normal(norBuf[index],
                            norBuf[index + 1],
                            norBuf[index + 2]);
                            
        float length = sqrt((normal.x() * normal.x()) + 
                            (normal.y() * normal.y()) + 
                            (normal.z() * normal.z()));
         
        norBuf[index] += normal.x()/length;
        norBuf[index + 1] += normal.y()/length;
        norBuf[index + 2] += normal.z()/length;
    }
}

void Shape::resize() {
    float minX, minY, minZ;
    float maxX, maxY, maxZ;
    float scaleX, scaleY, scaleZ;
    float shiftX, shiftY, shiftZ;
    float epsilon = 0.001;

    minX = minY = minZ = 1.1754E+38F;
    maxX = maxY = maxZ = -1.1754E+38F;

    //Go through all vertices to determine min and max of each dimension
    for (size_t v = 0; v < posBuf.size() / 3; v++) {
        if(posBuf[3*v+0] < minX) minX = posBuf[3*v+0];
        if(posBuf[3*v+0] > maxX) maxX = posBuf[3*v+0];

        if(posBuf[3*v+1] < minY) minY = posBuf[3*v+1];
        if(posBuf[3*v+1] > maxY) maxY = posBuf[3*v+1];

        if(posBuf[3*v+2] < minZ) minZ = posBuf[3*v+2];
        if(posBuf[3*v+2] > maxZ) maxZ = posBuf[3*v+2];
    }

    //From min and max compute necessary scale and shift for each dimension
    float maxExtent, xExtent, yExtent, zExtent;
    xExtent = maxX-minX;
    yExtent = maxY-minY;
    zExtent = maxZ-minZ;
    if (xExtent >= yExtent && xExtent >= zExtent) {
        maxExtent = xExtent;
    }
    if (yExtent >= xExtent && yExtent >= zExtent) {
        maxExtent = yExtent;
    }
    if (zExtent >= xExtent && zExtent >= yExtent) {
        maxExtent = zExtent;
    }
    scaleX = 2.0 /maxExtent;
    shiftX = minX + (xExtent/ 2.0);
    scaleY = 2.0 / maxExtent;
    shiftY = minY + (yExtent / 2.0);
    scaleZ = 2.0/ maxExtent;
    shiftZ = minZ + (zExtent)/2.0;

    //Go through all verticies shift and scale them
    for (size_t v = 0; v < posBuf.size() / 3; v++) {
        posBuf[3*v+0] = (posBuf[3*v+0] - shiftX) * scaleX;
        assert(posBuf[3*v+0] >= -1.0 - epsilon);
        assert(posBuf[3*v+0] <= 1.0 + epsilon);
        posBuf[3*v+1] = (posBuf[3*v+1] - shiftY) * scaleY;
        assert(posBuf[3*v+1] >= -1.0 - epsilon);
        assert(posBuf[3*v+1] <= 1.0 + epsilon);
        posBuf[3*v+2] = (posBuf[3*v+2] - shiftZ) * scaleZ;
        assert(posBuf[3*v+2] >= -1.0 - epsilon);
        assert(posBuf[3*v+2] <= 1.0 + epsilon);
    }
}

void Shape::init()
{
    cout << "start of Shape init GL_ERROR: " << glGetError() << endl;
    // Initialize the vertex array object
    glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);

    // Send the position array to the GPU
    glGenBuffers(1, &posBufID);
    glBindBuffer(GL_ARRAY_BUFFER, posBufID);
    glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);

    // Send the normal array to the GPU
    if(norBuf.empty()) {
        norBufID = 0;
    } else {
        glGenBuffers(1, &norBufID);
        glBindBuffer(GL_ARRAY_BUFFER, norBufID);
        glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);
    }

    // Send the texture array to the GPU
    if(texBuf.empty()) {
        texBufID = 0;
    } else {
        glGenBuffers(1, &texBufID);
        glBindBuffer(GL_ARRAY_BUFFER, texBufID);
        glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW);
    }

    // Send the element array to the GPU
    glGenBuffers(1, &eleBufID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, eleBuf.size()*sizeof(unsigned int), &eleBuf[0], GL_STATIC_DRAW);

    // Unbind the arrays
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    assert(glGetError() == GL_NO_ERROR);
}

void Shape::draw(const shared_ptr<Program> prog) const
{
    int h_pos, h_nor, h_tex;
    h_pos = h_nor = h_tex = -1;

    glBindVertexArray(vaoID);
    // Bind position buffer
    h_pos = prog->getAttribute("vertPos");
    GLSL::enableVertexAttribArray(h_pos);
    glBindBuffer(GL_ARRAY_BUFFER, posBufID);
    glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

    // Bind normal buffer
    h_nor = prog->getAttribute("vertNor");
    if(h_nor != -1 && norBufID != 0) {
        GLSL::enableVertexAttribArray(h_nor);
        glBindBuffer(GL_ARRAY_BUFFER, norBufID);
        glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
    }

    if (texBufID != 0) {	
        // Bind texcoords buffer
        h_tex = prog->getAttribute("vertTex");
        if(h_tex != -1 && texBufID != 0) {
            GLSL::enableVertexAttribArray(h_tex);
            glBindBuffer(GL_ARRAY_BUFFER, texBufID);
            glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0);
        }
    }

    // Bind element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);

    // Draw
    glDrawElements(GL_TRIANGLES, (int)eleBuf.size(), GL_UNSIGNED_INT, (const void *)0);

    // Disable and unbind
    if(h_tex != -1) {
        GLSL::disableVertexAttribArray(h_tex);
    }
    if(h_nor != -1) {
        GLSL::disableVertexAttribArray(h_nor);
    }
    GLSL::disableVertexAttribArray(h_pos);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
