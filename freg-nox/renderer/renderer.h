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
*along with FREG. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef RENDERER_H
#define RENDERER_H
#include "ph.h"

#include "texture.h"
#include "polygon_buffer.h"
#include "glsl_program.h"
#include "cubemap.h"
#include "font.h"
#include "frame_buffer.h"
#include "block_textures.h"
#include "texture_array.h"
#include "../thread.h"
#include "../world.h"
#include "../Shred.h"
//#include "../Player.h"
class Player;
#define MAX_GL_EXTENSIONS 512
#define R_SHRED_WIDTH ::shred_width
#define R_SHRED_HEIGHT ::height

#define MIN_SUN_HEIGHT 0.2618f

#define FREG_GL_MULTI_THREAD 0

#define WORLD_NORMAL_X      0
#define WORLD_NORMAL_MIN_X  1
#define WORLD_NORMAL_Y      2
#define WORLD_NORMAL_MIN_Y  3
#define WORLD_NORMAL_Z      4
#define WORLD_NORMAL_MIN_Z  5

const m_Vec3 day_fog( 0.8, 0.8, 1.0 );
const m_Vec3 night_fog( 0.15, 0.15, 0.1875 );
struct r_WorldVertex
{
    qint16 coord[3];
    quint8 tex_coord[2];
    quint8  normal_id;
    quint8 light;
    quint8 reserved[2];
};//12b struct


struct r_FontVertex
{
    float coord[2];
    quint16 tex_coord[2];
    quint8 color[3];
    quint8 tex_id;
};//16b struct

//Shred flags
#define SHRED_UPDATED 1
#define SHRED_VISIBLY 2
struct r_ShredInfo
{
    short longitude;
    short latitude;
    unsigned int quad_count;
    unsigned int quad_buffer_count;//������, ���������� ��� ������ � ��������� �������
    unsigned int index_buffer_offset;//�������� � ��������� ������� (� ������ )
    unsigned int flags;
    unsigned short  max_geometry_height,
    min_geometry_height;
    r_WorldVertex* vertex_buffer;//��������� �� ��������� ������( ������ ����������� ���������� ������� )
    unsigned char* visibly_information;//��������� �� ������, � ������� �������� ���������� � ��������� ������ ������
    Shred* shred;//pointer valid in 1 update

    r_ShredInfo *east_shred, *south_shred,
    *north_shred, *west_shred;

    bool updated, rebuilded, immediately_update;
    bool need_update_quads;
    bool visibly_information_is_valid;

    void GetQuadCount();
    void GetVisiblyInformation( );
    void BuildShred( r_WorldVertex* shred_vertices );
    void UpdateShred();
    void UpdateCube( short x0, short y0, short z0, short x1, short y1, short z1 );
    bool IsOnOtherSideOfPlane( m_Vec3 point, m_Vec3 normal );

    //���������� ������������ �������� ���������� �� ��� �� ������� �� ����� x,y
    unsigned short GetShredDistance( short x, short y );

};

class r_Renderer
{
public:

    inline void SetCamAngle( m_Vec3& a );
    inline void SetCamPosition( m_Vec3& p );
    inline void RotateCam( m_Vec3& r);

    inline int ViewportWidth()
    {
        return viewport_x;
    }
    inline int ViewportHeight()
    {
        return viewport_y;
    }


    r_Renderer( World* w,Player* p, int width, int height );
    void Initialize();
    void ShutDown();

    void UpdateBlock( short x, short y, short z );
    void MoveMap( int dir );
    void UpdateAll();
    void UpdateShred( short x, short y );
    void UpdateCube( short x0, short y0, short z0, short x1, short y1, short z1 );
    //void UpdateVisiblyInformation( short x, short y, short z );
public:
    void AddNotifyString( const char* str );
    void Draw();
    bool Initialized()
    {
        return renderer_initialized;
    }
private:

    void BuildWorld();
    void UpdateData();
    // ������� ������ � ���������
    void BuildShredList();
    //void UpdateShredGPUData( r_ShredInfo* shred );

    //hack for multithreading
public:
    static r_Renderer* current_renderer;
private:
    static void SUpdateTick();
    void UpdateTick();
    void SetupVertexBuffers();
    void DrawWorld();
    void DrawSky();
    void DrawHUD();
    void LoadTextures();

    QMutex gpu_data_mutex, host_data_mutex;

    struct
    {
        int update_interval;//in ms
        unsigned short double_update_interval_radius,
        quad_update_intrval_radius,
        octal_update_interval_radius;
    } r_Config;


    const World* world;
    const Player* player;
    r_UniversalThread update_thread;
    unsigned int update_count;

    short center_shred_longitude, center_shred_latitude;

    //r_Texture texture_atlas;
    r_TextureArray texture_array;
    r_CubeMap sky_cubemap;
    r_GLSLProgram world_shader, sky_shader;
    r_PolygonBuffer world_buffer, temp_buffer, sky_buffer;
    r_WorldVertex* vertex_buffer;
    quint32* indeces;

    unsigned char* visibly_information;
    unsigned short visibly_world_size[2];

    unsigned int vertex_buffer_size;//size of buffer
    unsigned int vertices_in_buffer;//real count of vertices in buffer
    unsigned int index_buffer_size;

    r_ShredInfo* shreds, *draw_shreds;
    r_ShredInfo** shreds_to_draw_list;
    quint32 *shreds_to_draw_indeces, * shreds_to_draw_quad_count;
    unsigned int shreds_to_draw_count;


    m_Mat4 view_matrix;
    m_Vec3 cam_position, cam_angle;
    float fov, z_near, z_far;
    unsigned short viewport_x, viewport_y;


    bool gl_texture_array_ext_enabled;
    unsigned int max_texture_units;

    void GetExtensionsStrings();
    bool GLExtensionSupported( char* ext_name );
    static char* extension_strings[ MAX_GL_EXTENSIONS ];
    static unsigned char extension_strings_hash[ MAX_GL_EXTENSIONS ];
    static unsigned int number_of_extension_string;


    bool renderer_initialized;
    bool out_of_vertex_buffer, out_of_index_buffer;

    unsigned int last_update_time;
    bool need_update_vertex_buffer;
    bool full_update, need_full_update_shred_list;

#if FREG_GL_MULTI_THREAD
    GLsync sync_object;
    unsigned int transmition_frame;
#endif

    //font section
#define R_LETTER_BUFFER_LEN 2048
    r_FontVertex* font_vertices;
    quint16* font_indeces;
    unsigned int letter_count;
    r_GLSLProgram text_shader;
    r_Font font;
    r_PolygonBuffer text_buffer;
    QMutex text_data_mutex;
    void StartUpText();
public:
    //����� ������. ���������� �������, ��� ����� ����������
    m_Vec3 AddText( float x, float y, const m_Vec3* color, unsigned int size,  const char* text, ... );
private:
    void DrawText();


#define R_NUMBER_OF_NOTIFY_LINES 9
#define R_NOTIFY_LINE_LENGTH	64
    char notify_log[ R_NUMBER_OF_NOTIFY_LINES ][ R_NOTIFY_LINE_LENGTH ];//4 ����� �� 64 �������
    unsigned int notify_log_first_line;//�����

    //shadows section
    r_FrameBuffer sun_shadow_map;// raw_sun_shadow_map;
    r_GLSLProgram shadow_shader;// shadowmap_transform_shader;
    r_PolygonBuffer fullscreen_quad;
    m_Mat4 shadow_matrix;
    void RenderShadows();
    m_Vec3 sun_vector;

    //perfomance counters section
    unsigned int last_fps_time;//����� ���������� ������ fps
    unsigned int last_frame_time;//����� ���������� �����
    unsigned int last_frame_number;//����� �����,����� ��������� ��� ��������� fps
    unsigned int frame_count;
    unsigned int fps;//���������� ������ �� ������� ���������
    unsigned int shreds_update_per_second, shreds_update_per_second_to_draw;
    float max_fps, min_fps, min_fps_to_draw, max_fps_to_draw;//������������ � ����������� fps �� ��� �����
    QTime startup_time;
    void CalculateFPS();

};


inline void r_ShredInfo::UpdateShred()
{
    updated= true;
    visibly_information_is_valid= false;
}

inline void r_Renderer::SUpdateTick()
{
    current_renderer->UpdateTick();
}
inline void r_Renderer::SetCamAngle( m_Vec3& a )
{
    cam_angle= a;
}
inline void r_Renderer::SetCamPosition( m_Vec3& p )
{
    cam_position= p;
}

inline void r_Renderer::RotateCam( m_Vec3& r)
{
    cam_angle+= r;
}
inline void r_Renderer::UpdateAll()
{
    full_update= true;
    unsigned short x, y;
    for( x= 0; x< visibly_world_size[0]; x++ )
    {
        for( y= 0; y< visibly_world_size[1]; y++ )
        {
            shreds[ x + visibly_world_size[0] * y ].visibly_information_is_valid= false;
        }
    }
}
#endif//RENDERER_H
