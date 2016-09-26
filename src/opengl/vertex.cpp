#include "saiga/opengl/vertex.h"


template<>
void VertexBuffer<Vertex>::setVertexAttributes(){
    glEnableVertexAttribArray( 0 );

    glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE, sizeof(Vertex), NULL );
}


template<>
void VertexBuffer<VertexN>::setVertexAttributes(){
    glEnableVertexAttribArray( 0 );
    glEnableVertexAttribArray( 1 );

    glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE, sizeof(VertexN), NULL );
    glVertexAttribPointer(1,3, GL_FLOAT, GL_FALSE, sizeof(VertexN), (void*) (3 * sizeof(GLfloat)) );
}

template<>
void VertexBuffer<VertexNT>::setVertexAttributes(){
    glEnableVertexAttribArray( 0 );
    glEnableVertexAttribArray( 1 );
    glEnableVertexAttribArray( 2 );

    glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE, sizeof(VertexNT), NULL );
    glVertexAttribPointer(1,3, GL_FLOAT, GL_FALSE, sizeof(VertexNT), (void*) (3 * sizeof(GLfloat)) );
    glVertexAttribPointer(2,2, GL_FLOAT, GL_FALSE, sizeof(VertexNT), (void*) (6 * sizeof(GLfloat)) );
}

template<>
void VertexBuffer<VertexNC>::setVertexAttributes(){
    glEnableVertexAttribArray( 0 );
    glEnableVertexAttribArray( 1 );
    glEnableVertexAttribArray( 2 );
    glEnableVertexAttribArray( 3 );

    glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE, sizeof(VertexNC), NULL );
    glVertexAttribPointer(1,3, GL_FLOAT, GL_FALSE, sizeof(VertexNC), (void*) (3 * sizeof(GLfloat)) );
    glVertexAttribPointer(2,3, GL_FLOAT, GL_FALSE, sizeof(VertexNC), (void*) (6 * sizeof(GLfloat)) );
    glVertexAttribPointer(3,3, GL_FLOAT, GL_FALSE, sizeof(VertexNC), (void*) (9 * sizeof(GLfloat)) );
}



bool Vertex::operator==(const Vertex &other) const {
    return position==other.position;
}

std::ostream &operator<<(std::ostream &os, const Vertex &vert){
    os<<vert.position;
    return os;
}

bool VertexN::operator==(const VertexN &other) const {
    return Vertex::operator==(other) && normal == other.normal;
}

std::ostream &operator<<(std::ostream &os, const VertexN &vert){
    os<<vert.position<<",";
    os<<vert.normal;
    return os;
}

bool VertexNT::operator==(const VertexNT &other) const {
    return VertexN::operator==(other) && texture == other.texture;
}

std::ostream &operator<<(std::ostream &os, const VertexNT &vert){
    os<<vert.position<<",";
    os<<vert.normal<<",";
    os<<vert.texture;
    return os;
}

bool VertexNC::operator==(const VertexNC &other) const {
    return VertexN::operator==(other) && color == other.color && data == other.data;
}

std::ostream &operator<<(std::ostream &os, const VertexNC &vert){
    os<<vert.position<<",";
    os<<vert.normal<<",";
    os<<vert.color<<",";
    os<<vert.data;
    return os;
}