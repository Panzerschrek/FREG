/*
*This file is part of FREG.
*
*FREG is free software: you can redistribute it and/or modify
*it under the terms of the GNU General Public License as published by
*the Free Software Foundation, either version 3 of the License, or
*(at your option) any later version.
*
*FREG is distributed in the hope that it will be useful,
*but WITHOUT ANY WARRANTY; without even the implied warranty of
*MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*GNU General Public License for more details.
*
*You should have received a copy of the GNU General Public License
*along with FREG. If not, see <http://www.gnu.org/licenses/>.*/
#ifndef TEXTURE_ARRAY_CPP
#define TEXTURE_ARRAY_CPP
#include "ph.h"

#include "texture_array.h"
#include "texture.h"

r_TextureArray::r_TextureArray()
{
    texture_data= NULL;
    id= 0xffffffff;
}

r_TextureArray::~r_TextureArray()
{
    if( texture_data != NULL )
        delete[] texture_data;

    if( id != 0xffffffff )
        glDeleteTextures( 1, &id );
}

int r_TextureArray::MoveOnGPU()
{
    if( id == 0xffffffff )
        glGenTextures( 1, &id );

    glBindTexture( GL_TEXTURE_2D_ARRAY, id );

    glTexParameteri( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );

    //GL_EXT_texture_filter_anisotropic
    //glTexParameteri( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4 );

    glTexImage3D( GL_TEXTURE_2D_ARRAY, 0, texture_type, width, height, layers, 0, texture_type, data_type, texture_data);

    if( texture_data != NULL )
        glGenerateMipmap( GL_TEXTURE_2D_ARRAY );
    r_Texture::ResetBinding();
    return 0;
}

void r_TextureArray::SetFiltration(GLuint min, GLuint mag )
{
    if( id == 0xffffffff ) return;

    glBindTexture( GL_TEXTURE_2D_ARRAY, id );

    glTexParameteri( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, mag );
    glTexParameteri( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, min );
    r_Texture::ResetBinding();
}
void r_TextureArray::GenerateMipmap()
{
    if( id == 0xffffffff ) return;
    Bind();
    glGenerateMipmap( GL_TEXTURE_2D_ARRAY );

}

void r_TextureArray::TextureLayer( int layer, unsigned char* data )
{
    if( layer > layers || id == 0xffffffff )
        return;
    Bind();
    glTexSubImage3D( GL_TEXTURE_2D_ARRAY, 0, 0, 0, layer, width, height,
                     1, texture_type, data_type, data );
}

void r_TextureArray::TextureData( int w, int h, int l, GLuint d_type, GLuint t_type, int bpp, void* data )
{
    if( texture_data != NULL )
        delete[] texture_data;

    texture_data= (unsigned char*)data;

    bits_per_pixel= bpp;
    texture_type= t_type;
    data_type= d_type;
    width= w;
    height= h;
    layers= l;
}

void r_TextureArray::DeleteFromRAM()
{
    if( texture_data != NULL )
        delete[] texture_data;
}

void r_TextureArray::DeleteFromGPU()
{
    if( id != 0xffffffff )
        glDeleteTextures( 1, &id );
}

void r_TextureArray::Bind( int unit )
{
    if( id == 0xffffffff )
        return;
    glActiveTexture( GL_TEXTURE0 + unit );
    glBindTexture( GL_TEXTURE_2D_ARRAY, id );

    r_Texture::ResetBinding();
}
#endif//TEXTURE_ARRAY_CPP
