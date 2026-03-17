/* ========================================================
 * @file        RenderSystem.cpp
 * @author      Wong Shi Yu Regine  | w.shiyuregine@digipen.edu
 * @co-author   Tan Jun Quan        | t.junquan@digipen.edu
 * @co-author   Kervin Ong Guan Cheng | kervinguancheng.ong@digipen.edu
 * @date        05 NOV 2025
 *
 * @brief
 *
 * Implements the RenderSystem, which is responsible for
 * drawing all visual components in the ECS framework.
 *
 * Responsibilities:
 * - Renders sprite components (static and animated).
 * - Renders shape components (rectangles and circles).
 * - Integrates with Animator for updating animated frames.
 * - Uses the central Render instance to issue draw calls.
 * - Supports debug drawing for physics (velocity vectors,
 *   AABBs, circle colliders, hotspots) controlled by ImGui.
 *
 * On shutdown, clears the texture cache to free GPU resources.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#include "FilePath.h"
#include "RenderSystem.h"
#include "Render.h"
#include "Animator.h"
#include "AudioSystem.h"
#include <Camera.h>
#include "LogicSystem.h"
#include "CollisionManager.h"
#include "CoreEngine.h"
#include "RenderComponent.h"
#include "SceneManager.h"
#include "SimpleAtlas.h"
#include "JsonLoad.h"
#include <iostream>

//need this to prevent name mangling 
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h> //NOTE: if cant work, link it in properties->linker->input->additional dependecies
#include <libavutil/imgutils.h>
}

static bool s_layerEnabled[4] = { true, true, true, true };
bool* RenderSystem::GetLayerEnabledArray() { return s_layerEnabled; }
 // ---------------------------------------------
 // @brief Temporary struct for per-frame draw queue
 // ---------------------------------------------
/*struct RenderEntry {
    Entity id;
    const Sprite::SpriteComponent* sprite;
    const TransformComponent* transform;
    float sortY;
};*/


//ffmpeg stuff to convert to RGBA
/*SwsContext* sws = nullptr;
uint8_t* dstData[4] = { nullptr, nullptr, nullptr, nullptr };
int dstLinesize[4] = { 0, 0, 0, 0 };
int dstW = 0, dstH = 0;
AVPixelFormat lastSrcFmt = AV_PIX_FMT_NONE;


//debug for ffmpeg
auto PrintErr = [](int err) {
    char buf[AV_ERROR_MAX_STRING_SIZE]{};
    av_strerror(err, buf, sizeof(buf));
    std::cout << buf;
    };

void TestOpenVideo() {

    //OPEN GL variables
    static GLuint videoTex = 0;
    static bool texInited = false;

    //FFmpeg varibles
    AVFormatContext* pFormatContext = nullptr;
    int result = avformat_open_input(
        &pFormatContext,
        (FilePath::ASSETS_FOLDER + "Intro.mp4"),
        nullptr,
        nullptr
    );

    //if successful print to console (sanity check)
    if (result == 0) {
        std::cout << "\n=====VIDEO MP4 DEBUG INFORMATION=====\nSuccessfully Opened video file\n";


        //now get stream info
        int info = avformat_find_stream_info(pFormatContext, nullptr);
        if (info == 0) {
            std::cout << "Successfully found stream info\n";
        }

        //Loop through streams
        int videoStreamIndex = -1;

        for (unsigned int i = 0; i < pFormatContext->nb_streams; i++) {
            AVStream* stream = pFormatContext->streams[i];

            if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                videoStreamIndex = i;
                break;
            }
        }

        if (videoStreamIndex == -1) {
            std::cout << "No video stream found\n";
            return;
        }

        //sanity checkpoint (make sure it matches the mp4 resolution in assets)=================================
        AVStream* videoStreamMeta = pFormatContext->streams[videoStreamIndex];

        std::cout << "<SANITY CHECK> Video Resolution ( width x height): "
            << videoStreamMeta->codecpar->width
            << " x "
            << videoStreamMeta->codecpar->height
            << std::endl;


        //next, we need find and open the decoder 
        AVStream* videoStream = pFormatContext->streams[videoStreamIndex];
        AVCodecParameters* codecParams = videoStream->codecpar;

        const AVCodec* codec = avcodec_find_decoder(codecParams->codec_id);

        //hello, ffpmeg do u have a decoder for this codec ID? if no have the video cannot be decoded 
        if (!codec) {
            std::cout << "Unsupported codec\n";
            return;
        }

        //if we have a decoder for it, ok, lets allocate a codec context
        //This will hold the decoder state
        AVCodecContext* codecCtx = avcodec_alloc_context3(codec);

        if (!codecCtx) {
            std::cout << "Failed to allocate codec context\n";
            return;
        }


        //if successful, next we shall copy the stream params into the context
        if (avcodec_parameters_to_context(codecCtx, codecParams) < 0) {
            std::cout << "Failed to copy codec parameters\n";
            return;
        }

        //After this, everything for the decoder is prepped and ready, we can open the decoder next
        // and ready to decode packets
        if (avcodec_open2(codecCtx, codec, nullptr) < 0) {
            std::cout << "Failed to open decoder\n";
            return;
        }

        //Now, we can read the packets from the stream and decode them into frames
        //First, lets allocate memory for AVPacket and AVFrame
        AVPacket* pPacket = av_packet_alloc();
        AVFrame* pFrame = av_frame_alloc();

        if (!pPacket || !pFrame) {
            std::cout << "Failed to allocate AVPacket/AVFrame\n";
            return;
        }


       

        //Next, we do the decode loop to turn the packets into frames
        while (av_read_frame(pFormatContext, pPacket) >= 0) {

            // Only process packets from the video stream
            if (pPacket->stream_index != videoStreamIndex) {
                av_packet_unref(pPacket);
                continue;
            }

            // Send the compressed packet to the decoder
            int ret = avcodec_send_packet(codecCtx, pPacket);

            // We can unref the packet right after sending, decoder will make its own copy
            av_packet_unref(pPacket);

            //Failed to send the packet to decoder
            if (ret < 0) {
                std::cout << "avcodec_send_packet failed: ";
                PrintErr(ret);
                std::cout << "\n";
                break;
            }


            //flush (maybe need? comment out first, since it was causing issues)
            //avcodec_send_packet(codecCtx, nullptr);
            // Flush decoder at end-of-stream

            // Receive as many frames as the decoder produces from that packet
            while (true) {

                ret = avcodec_receive_frame(codecCtx, pFrame);

                //success
                if (ret == 0) {
                   
                    //small test now to sanity check if its working
                    //YAYYY, Prints gotcha 901 times, matches my test mp4 (30 seconds at 30 fps)
                    //std::cout << "gotcha\n";

                    // --- IMPORTANT: NEXT STEP LATER ---
                    // We can do the manipulation of frames here, here is where we use the frames
                    
                
                    // Convert this frame to RGBA (sws_scale) and upload to OpenGL.

                    //Setup RGBA buffer and sws context and all
                    AVPixelFormat srcFmt = (AVPixelFormat)pFrame->format;
                    int w = pFrame->width;
                    int h = pFrame->height;

                    const char* fmtName = av_get_pix_fmt_name(srcFmt);



                    // (Re)initialize sws + output buffer if needed
                    if (!sws || w != dstW || h != dstH || srcFmt != lastSrcFmt) {

                        // free old output buffer
                        if (dstData[0]) {
                            av_freep(&dstData[0]);           // frees buffer from av_image_alloc
                            dstData[0] = nullptr;
                        }

                        // free old scaler
                        if (sws) {
                            sws_freeContext(sws);
                            sws = nullptr;
                        }

                        dstW = w;
                        dstH = h;
                        lastSrcFmt = srcFmt;

                        sws = sws_getContext(
                            w, h, srcFmt,
                            w, h, AV_PIX_FMT_RGBA,
                            SWS_BILINEAR,
                            nullptr, nullptr, nullptr
                        );

                        if (!sws) {
                            std::cout << "sws_getContext FAILED\n";
                            break; // don't return; let cleanup happen
                        }

                        int allocRet = av_image_alloc(dstData, dstLinesize, w, h, AV_PIX_FMT_RGBA, 1);
                        if (allocRet < 0) {
                            std::cout << "av_image_alloc FAILED: ";
                            PrintErr(allocRet);
                            std::cout << "\n";
                            break;
                        }

                    }


                    // Convert to RGBA fark
                    int outH = sws_scale(
                        sws,
                        pFrame->data,
                        pFrame->linesize,
                        0,
                        h,
                        dstData,
                        dstLinesize
                    );


                    if (outH <= 0) {
                        std::cout << "sws_scale FAILED/returned " << outH << "\n";
                        // keep going or break; your choice
                        break;
                    }



                    //(UP NEXT)
                    //Else... Next step will be OpenGL upload using dstData[0] and dstLinesize[0]
                    // Now rgbaData contains w*h pixels in RGBA format.
                    // Next step after this: upload rgbaData to an OpenGL texture.

                    //Set up and init buffers once for the OpenGL
                    if (!texInited) {
                        glGenTextures(1, &videoTex);
                        glBindTexture(GL_TEXTURE_2D, videoTex);

                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                        // Allocate GPU storage (no pixels yet)
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

                        texInited = true;
                    }


                    //Bind textures
                    glBindTexture(GL_TEXTURE_2D, videoTex);

                    // Handle stride (dstLinesize[0] may include padding)
                    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                    glPixelStorei(GL_UNPACK_ROW_LENGTH, dstLinesize[0] / 4); // RGBA = 4 bytes/pixel

                    glTexSubImage2D(
                        GL_TEXTURE_2D,
                        0,
                        0, 0,
                        w, h,
                        GL_RGBA,
                        GL_UNSIGNED_BYTE,
                        dstData[0]
                    );

                    // Reset to avoid affecting other texture uploads
                    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);









                    // When you're done using this frame's data, unref it:
                    av_frame_unref(pFrame);
                    continue;
                }


                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    // Decoder needs more packets before it can output more frames
                    break;
                }

                // Any other negative value is an actual error, try to get detailed error to help with debug
                std::cout << "avcodec_receive_frame error: ";
                PrintErr(ret);
                std::cout << "\n";
                break;
            }


        }





        //free the sws context and RGBA buffer 
        if (dstData[0]) {
            av_freep(&dstData[0]);
        }
        if (sws) {
            sws_freeContext(sws);
        }

        //Clean up texture
        if (videoTex) {
            glDeleteTextures(1, &videoTex);
            videoTex = 0;
            texInited = false;
        }


        //Cleanup previously allocated mem, to prevent mem leaks (i hope is in the right place)
        av_frame_free(&pFrame);
        av_packet_free(&pPacket);

        avcodec_free_context(&codecCtx);
        avformat_close_input(&pFormatContext);


    }

    //specify error if cannot open video
    else {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(result, errbuf, sizeof(errbuf));
        std::cout << "FFmpeg error: " << errbuf << std::endl;
    }
}*/

//AVFormatContext* pFormatContext = avformat_alloc_context();

Render& renderInstance = Render::GetInstance();

// Draws a health bar: dark background + colored fill scaled by ratio (0..1)
static void DrawHealthBar(const Vec2& pos, float width, float height, float ratio)
{
    // Background
    renderInstance.DrawRect(pos, width, height, { 0.2f, 0.2f, 0.2f });
    // Fill — shifts left so bar depletes from right
    float fillW = width * ratio;
    Vec2 fillPos = { pos.x - (width - fillW) * 0.5f, pos.y };
    float r = 1.f - ratio;
    float g = ratio;
    renderInstance.DrawRect(fillPos, fillW, height, { r, g, 0.f });
}

//Reorganise cutscene stuff below
struct CutscenePlayer {

    // FFmpeg
    AVFormatContext* fmt = nullptr;
    AVCodecContext* dec = nullptr;
    AVPacket* pkt = nullptr;
    AVFrame* frm = nullptr;
    int videoStreamIndex = -1;

    // swscale + RGBA buffer
    SwsContext* sws = nullptr;
    uint8_t* dstData[4] = { nullptr };
    int dstLinesize[4] = { 0 };
    int w = 0, h = 0;
    AVPixelFormat lastSrcFmt = AV_PIX_FMT_NONE;


    // OpenGL
    GLuint tex = 0;
    bool texInited = false;

    // Playback timing
    // start with hardcode (i follow the intro2 mp4 fps, 24fps)
    // see first if ltr on need use av_q2d(pFormatCtx->streams[videoStreamIndx]->r_frame_rate);
    double fps = 24.0;        
    double acc = 0.0;
    bool finished = false;

    std::string nextScene = "";

    bool Init(const char* path);
    void Update(double dt);   // decode and upload as needed
    void Render() const;            // draw texture
    void Shutdown();    //cleanup all once cutscene sectuon is done

};

static CutscenePlayer gCutscene;
//bool cutsceneActive;   // show cutscene instead of gameplay

bool CutscenePlayer::Init(const char* path)
{
    // Clean any previous state just in case 
    Shutdown();

    //helper lambda that uses the error handling that ffmpeg supports
    auto PrintErr = [](int err) {
        char buf[AV_ERROR_MAX_STRING_SIZE]{};
        av_strerror(err, buf, sizeof(buf));
        std::cout << buf;
        };

    // Open container
    int r = avformat_open_input(&fmt, path, nullptr, nullptr);
    if (r < 0) {
        std::cout << "avformat_open_input failed: ";
        PrintErr(r);
        std::cout << "\n";
        return false;
    }

    // Read stream metadata (needed to locate streams/codecs)
    r = avformat_find_stream_info(fmt, nullptr);
    if (r < 0) {
        std::cout << "avformat_find_stream_info failed: ";
        PrintErr(r);
        std::cout << "\n";
        return false;
    }

    // Find the video stream index
    videoStreamIndex = -1;
    for (unsigned i = 0; i < fmt->nb_streams; ++i) {
        AVStream* s = fmt->streams[i];
        if (s && s->codecpar && s->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = static_cast<int>(i);
            break;
        }
    }

    if (videoStreamIndex < 0) {
        std::cout << "Init failed: no video stream found in " << path << "\n";
        return false;
    }

    // Find decoder for that stream
    AVCodecParameters* par = fmt->streams[videoStreamIndex]->codecpar;
    const AVCodec* codec = avcodec_find_decoder(par->codec_id);
    if (!codec) {
        std::cout << "Init failed: unsupported codec id = " << par->codec_id << "\n";
        return false;
    }

    // Create decoder context
    dec = avcodec_alloc_context3(codec);
    if (!dec) {
        std::cout << "Init failed: avcodec_alloc_context3 returned null\n";
        return false;
    }

    // Copy stream parameters into decoder context
    r = avcodec_parameters_to_context(dec, par);
    if (r < 0) {
        std::cout << "avcodec_parameters_to_context failed: ";
        PrintErr(r);
        std::cout << "\n";
        return false;
    }

    // Open decoder
    r = avcodec_open2(dec, codec, nullptr);
    if (r < 0) {
        std::cout << "avcodec_open2 failed: ";
        PrintErr(r);
        std::cout << "\n";
        return false;
    }

    // Allocate reusable packet + frame
    pkt = av_packet_alloc();
    frm = av_frame_alloc();
    if (!pkt || !frm) {
        std::cout << "Init failed: av_packet_alloc/av_frame_alloc returned null\n";
        return false;
    }

    // print info sanity check for myself
    std::cout << "[CUTSCENE INIT]: SUCCESS\n";
    std::cout << "Path: " << path << "\n";
    std::cout << "VideoStreamIndex: " << videoStreamIndex << "\n";
    std::cout << "Codec: " << (codec->long_name ? codec->long_name : codec->name) << "\n";

    finished = false;
    texInited = false;

    return true;
}


void CutscenePlayer::Update(double dt) {

    if (finished || !fmt) {
        return;
    }

    acc += dt;
    const double frameTime = 1.0 / fps;

    while (acc >= frameTime) {
        acc -= frameTime;

        // decode exactly one frame and upload it
        // loop reading packets until you get a frame
        bool gotFrame = false;

        while (!gotFrame) {

            int r = av_read_frame(fmt, pkt);

            if (r < 0) { 
                finished = true; 
                return; 
            }

            if (pkt->stream_index != videoStreamIndex) {
                av_packet_unref(pkt);
                continue;
            }

            r = avcodec_send_packet(dec, pkt);
            av_packet_unref(pkt);

            if (r < 0) { 
                finished = true; 
                return; 
            }

            while (true) {

                r = avcodec_receive_frame(dec, frm);

                if (r == 0) {
                    gotFrame = true;

                    // init sws/rgba/texture on first frame
                    if (!texInited) {
                        w = frm->width; h = frm->height;
                        sws = sws_getContext(w, h, (AVPixelFormat)frm->format, w, h, AV_PIX_FMT_RGBA,
                            SWS_BILINEAR, nullptr, nullptr, nullptr);
                        av_image_alloc(dstData, dstLinesize, w, h, AV_PIX_FMT_RGBA, 1);

                        glGenTextures(1, &tex);
                        glBindTexture(GL_TEXTURE_2D, tex);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

                        texInited = true;


                    }

                    //sws_scale(sws, frm->data, frm->linesize, 0, h, dstData, dstLinesize);
                    
                    // need to flip the image, cause ffmpeg and opengl texture origin mismatch
                    // flip vertically during conversion (FFmpeg top-left map to GL bottom-left)
                    uint8_t* dstSlice[4] = { dstData[0], dstData[1], dstData[2], dstData[3] };
                    int      dstStride[4] = { dstLinesize[0], dstLinesize[1], dstLinesize[2], dstLinesize[3] };

                    // Start writing at last row, and walk upward by using negative stride
                    dstSlice[0] = dstData[0] + (h - 1) * dstLinesize[0];
                    dstStride[0] = -dstLinesize[0];

                    sws_scale(sws, frm->data, frm->linesize, 0, h, dstSlice, dstStride);


                    glBindTexture(GL_TEXTURE_2D, tex);
                    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                    glPixelStorei(GL_UNPACK_ROW_LENGTH, dstLinesize[0] / 4);
                    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, dstData[0]);
                    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

                   


                    av_frame_unref(frm);
                    break;
                }

                if (r == AVERROR(EAGAIN) || r == AVERROR_EOF) {
                    break;
                }

                finished = true; 
                return;

            }
        }
    }
}



//draw texture every frame, put in render update?? idk uhhh
void CutscenePlayer::Render() const {
    if (!texInited) {
        return;
    }

    renderInstance.DrawLetterboxedTexture(tex);
}


//cleanup for cutscene
void CutscenePlayer::Shutdown() {
    if (dstData[0]) {
        av_freep(&dstData[0]);
        dstData[0] = nullptr;
		dstLinesize[0] = 0;
    }

    if (sws) {
        sws_freeContext(sws);
        sws = nullptr;
    }

    if (frm) {
        av_frame_free(&frm);
        frm = nullptr;
    }

    if (pkt) {
        av_packet_free(&pkt);
        pkt = nullptr;
    }

    if (dec) {
        avcodec_free_context(&dec);
        dec = nullptr;
    }

    if (fmt) {
        avformat_close_input(&fmt);
        fmt = nullptr;
    }

    if (tex) {
        glDeleteTextures(1, &tex);
    }

    tex = 0;
    texInited = false;
    //nextScene = "";
}

static std::string s_cutscenePath{}; // Static cutscene path









//Render& renderInstance = Render::GetInstance();
std::vector<InstanceData> batch; //added for instancing
static std::vector<GLuint> s_backgroundTextures;
static std::vector<std::string> s_backgroundTexturePaths;




// Pixels to units conversion, assigned values on initialization of rendersystem
float RenderSystem::pixelsPerUnit;
float RenderSystem::unitsPerPixel;

void RenderSystem::AddBackgroundTexture(GLuint tex, const std::string& path) {
    s_backgroundTextures.push_back(tex);
    s_backgroundTexturePaths.push_back(path);
}

std::vector<std::string> RenderSystem::GetBackgroundTexturePaths() {
    return s_backgroundTexturePaths;
}

void RenderSystem::SetCutscenePath(const std::string& path) {
    s_cutscenePath = path;
    // If a cutscene is currently loaded, reinit with the new video
    if (gCutscene.fmt) {
        if (!gCutscene.Init(s_cutscenePath.c_str())) {
            // make sure everything is cleaned up so Update dont crash
            gCutscene.Shutdown();
            gCutscene.finished = true;
        }
        gCutscene.acc = 0.0;
    }
}

std::string RenderSystem::GetCutscenePath() {
	return s_cutscenePath;
}

void RenderSystem::SetCutsceneFPS(double fps) {
    gCutscene.fps = (fps);
}

double RenderSystem::GetCutsceneFPS() {
    return (gCutscene.fps);
}

void RenderSystem::ClearBackgroundTextures() {
    s_backgroundTextures.clear();
    s_backgroundTexturePaths.clear();
}

void RenderSystem::SetNextScene(const std::string& path) {
    gCutscene.nextScene = path;
}

std::string RenderSystem::GetNextScene() {
    return gCutscene.nextScene;
}

//extern Camera mainCamera;
//Camera& mainCamera = Camera::GetInstance();

// NEW RENDERCOMPONENT draw overloads
void DrawItem(const Sprite::SpriteComponent& spriteComp, const TransformComponent& transComp) {
    const Image& img = spriteComp.Get_Image();

    Vec2 worldSize = { img.widthPixels * RenderSystem::unitsPerPixel, img.heightPixels * RenderSystem::unitsPerPixel };

    Vec2 scale = worldSize * spriteComp.Get_Scale() * transComp.scale;
    //Vec2 scaledSize = { img.widthPixels * spriteComp.Get_Scale().x, img.heightPixels * spriteComp.Get_Scale().y };

    glm::vec4 uv = { 0.f, 1.f, 1.f, 0.f };

    if (spriteComp.GetFlipX()) { std::swap(uv.x, uv.z); }
    if (spriteComp.GetFlipY()) { std::swap(uv.y, uv.w); }

    renderInstance.SubmitSpriteBatchUV(
        img,
        transComp.pos,
        scale,
        uv,
        transComp.rot + spriteComp.Get_Rotation()
    );

    //renderInstance.SubmitSpriteBatch(
    //    img,
    //    transComp.pos,
    //    scale,
    //    transComp.rot + spriteComp.Get_Rotation()
    //);
}

void DrawItem(const Sprite::SpriteSheet& spriteSheet, const TransformComponent& transComp) {
    const Image& img = spriteSheet.Sprite().Get_Image();

    Vec2 frameSize = { spriteSheet.u1 - spriteSheet.u0, spriteSheet.v1 - spriteSheet.v0 };
    Vec2 worldSize = frameSize * RenderSystem::unitsPerPixel;

    Vec2 scale = worldSize * spriteSheet.Sprite().Get_Scale() * transComp.scale;

    glm::vec4 uv(spriteSheet.u0 / img.widthPixels,
        spriteSheet.v1 / img.heightPixels,
        spriteSheet.u1 / img.widthPixels,
        spriteSheet.v0 / img.heightPixels
    );

    if (spriteSheet.Sprite().GetFlipX()) { std::swap(uv.x, uv.z); }
    if (spriteSheet.Sprite().GetFlipY()) { std::swap(uv.y, uv.w); }

    renderInstance.SubmitSpriteBatchUV(
        img,
        transComp.pos,
        scale,
        uv,
        transComp.rot + spriteSheet.Sprite().Get_Rotation()
    );

}

void DrawItem(const ShapeComponent& shapeComp, const TransformComponent& transComp) {
    Colour3 color = { shapeComp.color.x, shapeComp.color.y, shapeComp.color.z };
    switch (shapeComp.shape)
    {
    case SHAPE_RECTANGLE:
    {
        renderInstance.DrawRect(
            transComp.pos,
            shapeComp.dimensions.x * transComp.scale.x,
            shapeComp.dimensions.y * transComp.scale.y,
            color,
            transComp.rot
        );
    }
    break; // end SHAPE_RECTANGLE

    case SHAPE_CIRCLE:
    {
        float radius = (shapeComp.dimensions.x + shapeComp.dimensions.y) / 4.0f;
        renderInstance.DrawCircle(
            transComp.pos,
            radius * transComp.scale.x,
            color,
            32
        );
    }
    break; // end SHAPE_CIRCLE

    default:
        break;
    }
}
// NEW RENDERCOMPONENT draw overloads end

void RenderSystem::Initialize() {
    // ? 
    std::cout << "[RenderSystem] Initializing..." << std::endl;

    Render& renderer = Render::GetInstance();
    renderer.Init();        // quad VAO/VBO + sprite shader
    //renderer.InitBatch();
    // Initialize your offscreen framebuffer for Game View
    renderer.InitSceneFramebuffer(GLApp::width, GLApp::height);

    std::cout << "[RenderSystem] Scene framebuffer created ("
        << GLApp::width << "x" << GLApp::height << ")\n";

    BehaviourFCT camFollow;
    camFollow.Init = [](Entity e) { CamFollowScript::Start(e); };
    camFollow.Update = [](Entity e, float dt) { CamFollowScript::Update(e, dt); };
    camFollow.End = [](Entity e) {CamFollowScript::End(e); };
    LogicSystem::Get().RegisterScript("CamTarget", camFollow);

    pixelsPerUnit = 100.f;
    unitsPerPixel = 1.f / pixelsPerUnit;

    EventBus::Get().Subscribe<SceneLoadEvent>([](const SceneLoadEvent& e) {
        if (e.sceneType == ST_CUTSCENE) {
            if (!gCutscene.Init(s_cutscenePath.c_str())) {
                gCutscene.Shutdown();
                gCutscene.finished = true;
            }
        }
        else {
            gCutscene.Shutdown();
        }
		}
    );

    //atlas testing below 
    /*SimpleAtlas atlas;
    SimpleAtlasBuilder builder;

    builder.addImage("rock", (FilePath::ASSETS_FOLDER + "Rock_2048px.png"));
    builder.addImage("rock_fight", (FilePath::ASSETS_FOLDER + "Rock_FightingPose_2048px.png"));
    builder.addImage("ghoul", (FilePath::ASSETS_FOLDER + "Ghoul_2048px.png"));
    builder.build(atlas, 4000);

    std::cout << "\n=== ATLAS BUILD RESULT ===\n";
    std::cout << "Atlas Texture ID: " << atlas.textureID << "\n";
    std::cout << "Atlas Size: " << atlas.width << " x " << atlas.height << "\n";

    for (auto& [key, sp] : atlas.sprites)
    {
        std::cout << "Sprite: " << key << "\n";
        std::cout << "  UV: (" << sp.u0 << "," << sp.v0
            << ") -> (" << sp.u1 << "," << sp.v1 << ")\n";
        std::cout << "  Size: " << sp.width << "x" << sp.height << "\n";
    }*/



    //TestOpenVideo();

    //CutscenePlayer gCutscene;
    //gCutscene.Init((FilePath::ASSETS_FOLDER + "IntroCutscene.mp4"));
    //cutsceneActive = true;
    //SceneManager::Get().SetCutscenePlaying(true);

    
}

void RenderSystem::Update(float dt) {
    
    //Camera::GetInstance().UpdateMatrices();

    Camera::GetInstance().UpdateCamera();

    Render& renderer = Render::GetInstance();
    
    bool camcheck = renderer.useCamera;

 

   



#ifndef NO_IMGUI
    if (ImGuiSystem::guiEnabled) {
        renderer.BeginSceneCapture(); 
        renderer.ClearBackground({ 0.1f, 0.1f, 0.1f, 1.0f });
    }
    else 
#endif
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, GLApp::width, GLApp::height);
        //renderer.ApplyLetterboxViewport();
        renderer.ClearBackground({ 0.0f, 0.0f, 0.0f, 1.0f }); // clear main framebuffer
    }

    static bool wasPaused = false;

    // New pause logic driven by SceneManager
    SceneManager& sm = SceneManager::Get();

    // Simulation is paused whenever we are NOT in play mode
    bool simPaused = !sm.IsPlaying();

    if (simPaused)
    {
        wasPaused = true;
    }
    else if (wasPaused)
    {
        // Just unpaused → reset velocity lines so they snap correctly
        DebugDraw::ClearVelocityCache();
        wasPaused = false;
    }

    // Menu scenes: use virtual 1920x1080 projection so entity positions (stored in virtual
    // space) render correctly
    // Game/other scenes: use actual window dimensions so the camera covers the right area.
    bool isMenuScene = (SceneManager::Get().GetSceneType() == ST_MENU);
    Render::GetInstance().UpdateProjection(isMenuScene);
   // renderer.ClearBackground({ 0.05f, 0.0f, 0.1f, 1.0f });
    // ===== DRAW BACKGROUND IF SET =====

    /*for (GLuint tex : s_backgroundTextures)
        renderer.DrawFullscreenTexture(tex, true);*/

        // ===== DRAW BACKGROUND IF SET =====

    // figure out current game state 
   // GameState currState = GSM_CurrentState();  


    //if (SceneManager::Get().GetCurrentSceneName() == "MainMenu")
    //{
    //    // Letterboxed menu background
    //    renderer.ClearBackground({ 0.0f, 0.0f, 0.0f, 1.0f });

    //    for (GLuint tex : s_backgroundTextures)
    //    {
    //        renderer.DrawLetterboxedTexture(tex);
    //    }
    //}
    //else
    //{
    //    // Gameplay or editor: fullscreen world background
    //    for (GLuint tex : s_backgroundTextures)
    //    {
    //        renderer.DrawFullscreenTexture(tex, true);
    //    }
    //}

    switch (SceneManager::Get().GetSceneType())
    {
    case ST_MENU:
    {

        renderer.useCamera = false;
        // Letterboxed menu background
        renderer.ClearBackground({ 0.0f, 0.0f, 0.0f, 1.0f });

        for (GLuint tex : s_backgroundTextures)
        {
            renderer.DrawLetterboxedTexture(tex);
        }
    }
        break; // end ST_MENU

    case ST_GAME:
    {
        bool captureScene = false;
#ifndef NO_IMGUI
        captureScene = ImGuiSystem::guiEnabled;
#endif
        renderer.useCamera = true;
        for (GLuint tex : s_backgroundTextures)
        {
            renderer.DrawFullscreenTexture(tex, true);
        }
    }
        break; // end ST_GAME

    case ST_CUTSCENE:
    {
        bool captureScene = false;
#ifndef NO_IMGUI
        captureScene = ImGuiSystem::guiEnabled;
#endif

        // TRACK FADE-OUT STATE FOR CUTSCENE MUSIC TO FADE OUT
        static bool cutsceneFadingOut = false;
        static double fadeOutTimer = 0.0;
        static const double FADE_OUT_DURATION = 2.0; // 2 second fade

        if (!gCutscene.finished)
        {
            gCutscene.Update(dt);

            if (captureScene) {
                renderer.BeginSceneCapture();
                renderer.ClearBackground({ 0,0,0,1 });
                renderer.useCamera = false;
                renderer.SetProjectionType(ProjectionType::UI);
                gCutscene.Render();
                renderer.EndSceneCapture();
            }

            // draw to the real screen too
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, GLApp::width, GLApp::height);
            renderer.ClearBackground({ 0,0,0,1 });
            renderer.useCamera = false;
            renderer.SetProjectionType(ProjectionType::UI);
            gCutscene.Render();

            return;
        }
        else
        {
            // start audio fade-out WHEN CUTSCENE OVER
            if (!cutsceneFadingOut) {
                std::cout << "[RenderSystem] Cutscene finished, starting audio fade-out\n";

                // Trigger audio fade-out in AudioManager
                AudioManager::Get().FadeOutAndStop(FADE_OUT_DURATION);

                cutsceneFadingOut = true;
                fadeOutTimer = 0.0;
            }

            // WAIT for fade-out to complete
            if (cutsceneFadingOut) {
                fadeOutTimer += dt;

                // KEEP rendering the last frame of the cutscene while fading
                if (captureScene) {
                    renderer.BeginSceneCapture();
                    renderer.ClearBackground({ 0,0,0,1 });
                    renderer.useCamera = false;
                    renderer.SetProjectionType(ProjectionType::UI);
                    gCutscene.Render();
                    renderer.EndSceneCapture();
                }

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glViewport(0, 0, GLApp::width, GLApp::height);
                renderer.ClearBackground({ 0,0,0,1 });
                renderer.useCamera = false;
                renderer.SetProjectionType(ProjectionType::UI);
                gCutscene.Render();

                // FADE COMPLETE, LOAD NEXT SCENE
                if (fadeOutTimer >= FADE_OUT_DURATION) {
                    std::cout << "[RenderSystem] Fade-out complete, loading next scene\n";

                    SceneManager::Get().SetCutscenePlaying(false);
                    gCutscene.Shutdown();

                    // reset fade
                    cutsceneFadingOut = false;
                    fadeOutTimer = 0.0;

                    SceneManager::Get().QueueLoadScene((FilePath::SAVES_FOLDER) + GetNextScene() + ".json");
                }

                return;
            }
        }

        for (GLuint tex : s_backgroundTextures)
        {
            renderer.DrawFullscreenTexture(tex, true);
        }
    }
    break; // end ST_CUTSCENE

    default:
        break;
    }           // end switch scene type

//    if (SceneManager::Get().GetSceneType() == ST_MENU)
//    {
//        // Letterboxed menu background
//        renderer.ClearBackground({ 0.0f, 0.0f, 0.0f, 1.0f });
//
//        for (GLuint tex : s_backgroundTextures)
//        {
//            renderer.DrawLetterboxedTexture(tex);
//        }
//    }
//    else // ST_GAME
//    {
//
//        //beginning cutscene
//        //Only set it once to true
//        static bool startedCutscene = false;
//        if (SceneManager::Get().GetSceneType() == ST_GAME && !startedCutscene) {
//            SceneManager::Get().SetCutscenePlaying(true);
//            startedCutscene = true;
//        }
//        /*
//        if (SceneManager::Get().IsCutscenePlaying())
//        {
//
//            #ifndef NO_IMGUI
//            if (ImGuiSystem::guiEnabled) {
//                renderer.BeginSceneCapture();                 // binds scene FBO + sets viewport
//                renderer.ClearBackground({ 0.f,0.f,0.f,1.f });  // clear scene FBO
//            }
//            else
//            #endif
//            {
//                glBindFramebuffer(GL_FRAMEBUFFER, 0);
//                glViewport(0, 0, GLApp::width, GLApp::height);
//                renderer.ClearBackground({ 0.f,0.f,0.f,1.f });  // clear backbuffer
//            }
//
//
//            gCutscene.Update(dt);
//
//            // renderer.ApplyLetterboxViewport();
//
//            renderer.ClearBackground({ 0.f, 0.f, 0.f, 1.f });
//
//            // Draw the cutscene in a projection/state that won't be overwritten
//            // Important: avoid camera projection for this.
//            auto oldCam = renderer.useCamera;
//            renderer.useCamera = false;
//            renderer.SetProjectionType(ProjectionType::UI);
//
//            gCutscene.Render(); // calls DrawLetterboxedTexture(tex)
//
//            renderer.SetProjectionType(ProjectionType::WORLD);
//            renderer.useCamera = oldCam;
//
//
//            #ifndef NO_IMGUI
//            // If your pipeline expects EndSceneCapture() for the editor view, do it here:
//            if (ImGuiSystem::guiEnabled) {
//                renderer.EndSceneCapture();
//            }
//            #endif
//
//            //End condition
//            if (gCutscene.finished)
//            {
//                //cutsceneActive = false;
//                SceneManager::Get().SetCutscenePlaying(false);
//                gCutscene.Shutdown(); // or keep texture if you want fade-out etc.
//
//            }
//
//            return; //IMPORTANT: don't run the rest of RenderSystem
//        }*/
//
//        bool captureScene = false;
//#ifndef NO_IMGUI
//        captureScene = ImGuiSystem::guiEnabled;
//#endif
//
//        if (sm.IsCutscenePlaying()) {
//            //gCutscene.Update(dt);
//
//            //if (captureScene) {
//            //    renderer.BeginSceneCapture();        // binds scene FBO
//            //    renderer.ClearBackground({ 0,0,0,1 });
//            //    renderer.useCamera = false;
//            //    renderer.SetProjectionType(ProjectionType::UI);
//            //    gCutscene.Render();
//            //    renderer.EndSceneCapture();
//            //}
//
//            //// draw to the real screen too
//            //glBindFramebuffer(GL_FRAMEBUFFER, 0);
//            //glViewport(0, 0, GLApp::width, GLApp::height);
//            //renderer.ClearBackground({ 0,0,0,1 });
//            //renderer.useCamera = false;
//            //renderer.SetProjectionType(ProjectionType::UI);
//            //gCutscene.Render();
//
//
//            //if (gCutscene.finished)
//            //{
//            //    //cutsceneActive = false;
//            //    SceneManager::Get().SetCutscenePlaying(false);
//            //    gCutscene.Shutdown(); // or keep texture if you want fade-out etc.
//
//            //    SceneManager::Get().QueueLoadScene((FilePath::ASSETS_FOLDER + "saves/level1.json")); //wacky fix;
//
//
//            //}  
//
//            //return;
//        }
//
//
//        // Gameplay or editor: fullscreen world background
//        for (GLuint tex : s_backgroundTextures)
//        {
//            renderer.DrawFullscreenTexture(tex, true);
//        }
//    }

   

    //if (!SceneManager::Get().IsPlaying()) {
    //    // main menu / editor mode
    //    for (GLuint tex : s_backgroundTextures)
    //        renderer.DrawLetterboxedTexture(tex); // OR letterbox version
    //}
    //else {
    //    // gameplay
    //    for (GLuint tex : s_backgroundTextures)
    //        renderer.DrawLetterboxedTexture(tex);
    //}

    // Use the camera for all subsequent draw calls (ok why this not workinGGGGGGGG AJBFEEUFBUJFBFDJSBJF)
    //mainCamera.UpdateMatrices();
    // Camera::GetInstance().UpdateMatrices();


    //glm::mat4 viewProj = mainCamera.GetViewProjection();
    glm::mat4 viewProj = Camera::GetInstance().GetViewProjection();

    renderInstance.SetUniformMat4(renderInstance.GetSpriteShader(), "uProjection", viewProj);
    


    /*glm::mat4 viewProj = mainCamera.GetViewProjection();
    GLuint shader = renderInstance.GetSpriteShader();
    glUseProgram(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, "uProjection"), 1, GL_FALSE, &viewProj[0][0]);*/

    //glUseProgram(renderInstance.GetSpriteShader());
    //glUniformMatrix4fv(glGetUniformLocation(renderInstance.GetSpriteShader(), "uProjection"), 1, GL_FALSE, &viewProj[0][0]);



// ======================================================
// trying batch-based rendering with layer and y-axis sorting
// my thought process is: 
// added a RenderLayerComponent for each entity (defaulted to gameobject layer unless specified inside entitycreation or json)
// group entities by layers, push them into each of their respective specified containers
// sort the game object layer entities based on their y coords from 
// reusable function drawlayer will be used to draw each layer
// ======================================================
    // renderInstance.BeginBatch();

    //struct RenderItem { //iidentify sprite or shape
    //    Entity id;
    //    bool isSprite;
    //};

    ////containers that contain entities for each layer
    //std::vector<RenderItem> backgroundEntities;
    //std::vector<RenderItem> gameEntities;
    //std::vector<RenderItem> foregroundEntities; //for sepcial effects? particles?
    //std::vector<RenderItem> uiEntities;


    ////Categorize entities by layer
    //for (auto& [entID, spriteComp] : m_registry.Get_ComponentMap<Sprite::SpriteComponent>())
    //{
    //    RenderLayer layer = LAYER_GAMEOBJECT; // default, if not specified 

    //    if (m_registry.Has_Component<RenderLayerComponent>(entID)) {
    //        layer = m_registry.Get_Component<RenderLayerComponent>(entID).layer;
    //    }

    //    RenderItem item{ entID, true };

    //    switch (layer)
    //    {
    //    case LAYER_BACKGROUND:
    //        backgroundEntities.push_back(item);
    //        break;
    //    case LAYER_GAMEOBJECT:
    //        gameEntities.push_back(item);
    //        break;
    //    case LAYER_FOREGROUND:
    //        foregroundEntities.push_back(item);
    //        break;
    //    case LAYER_UI:
    //        uiEntities.push_back(item);
    //        break;
    //    }
    //}

    ////Handle the shape components
    //for (auto& [entID, shapeComp] : m_registry.Get_ComponentMap<ShapeComponent>()) {

    //    RenderLayer layer = LAYER_GAMEOBJECT; //default give them layer_gameobject, if want change need explicitly add layer comp to entitycreation.cpp

    //    if (m_registry.Has_Component<RenderLayerComponent>(entID)) {
    //        layer = m_registry.Get_Component<RenderLayerComponent>(entID).layer;
    //    }

    //    RenderItem item{ entID, false };

    //    switch (layer)
    //    {
    //    case LAYER_BACKGROUND:
    //        backgroundEntities.push_back(item);
    //        break;
    //    case LAYER_GAMEOBJECT:
    //        gameEntities.push_back(item);
    //        break;
    //    case LAYER_FOREGROUND:
    //        foregroundEntities.push_back(item);
    //        break;
    //    case LAYER_UI:
    //        uiEntities.push_back(item);
    //        break;
    //    }
    //}

    // new rendercomp
#pragma region new rendercomp

    std::vector<std::pair<Entity,RenderComponent>> bg_list;
    std::vector<std::pair<Entity,RenderComponent>> go_list;
    std::vector<std::pair<Entity,RenderComponent>> fg_list; //for sepcial effects? particles?
    std::vector<std::pair<Entity,RenderComponent>> ui_list;

    // NEW RENDERCOMPONENT
    for (auto& [entID, renderComp] : m_registry.Get_ComponentMap<RenderComponent>()) {
        renderComp.UpdateData(dt);

        switch (renderComp.GetRenderLayer())
        {
        case LAYER_BACKGROUND:
            bg_list.push_back({ entID,renderComp });
            break;
        case LAYER_GAMEOBJECT:
            go_list.push_back({ entID,renderComp });
            break;
        case LAYER_FOREGROUND:
            fg_list.push_back({ entID,renderComp });
            break;
        case LAYER_UI:
            ui_list.push_back({ entID,renderComp });
            break;
        }
    }

    //Sort the entities with GAMEOBJECT layer by Y-position
    //std::sort(gameEntities.begin(), gameEntities.end(),
    //    [&](const RenderItem& a, const RenderItem& b) {
    //        const auto& ta = m_registry.Get_Component<TransformComponent>(a.id);
    //        const auto& tb = m_registry.Get_Component<TransformComponent>(b.id);
    //        return ta.pos.y > tb.pos.y; // Higher Y = further back (drawn first)
    //    });

    // SORT ENTITIES FOR NEW RENDERCOMPONENT
    std::sort(go_list.begin(), go_list.end(),
        [&](const std::pair<Entity, RenderComponent>& a, const std::pair<Entity, RenderComponent>& b) {
            const auto& ta = m_registry.Get_Component<TransformComponent>(a.first);
            const auto& tb = m_registry.Get_Component<TransformComponent>(b.first);
            return ta.pos.y > tb.pos.y; // Higher Y = further back (drawn first)
        });

    auto drawLayer = [&](const std::vector<std::pair<Entity, RenderComponent>>& renderList)
        {
            for (const auto& [entID, renderComp] : renderList)
            {
                if (!m_registry.Has_Component<TransformComponent>(entID)) { continue; }
                TransformComponent& transComp = m_registry.Get_Component<TransformComponent>(entID);

                // update rendercomponent
                switch (renderComp.GetRenderType())
                {
                case RT_SHAPE:
                {
                    DrawItem(renderComp.GetShape(), transComp);
                }
                break; // end RT_SHAPE

                case RT_IMAGE:
                {
                    DrawItem(renderComp.GetSprite(), transComp);
                }
                break; // end RT_IMAGE

                case RT_SPRITESHEET:
                {
                    DrawItem(renderComp.GetSpriteSheet(), transComp);
                }
                break; // end RT_SPRITESHEET

                default:
                    break;
                }

                // draw de thing
                /*std::visit([&](auto& renderdata)
                    { DrawItem(renderdata, transComp); }, renderComp.m_renderData);*/
            }
        };
#pragma endregion new rendercomp

    //Draw a list of entities
    //auto DrawLayer = [&](const std::vector<RenderItem>& entities)
    //    {
    //        for (const RenderItem& item : entities)
    //        {
    //            Entity entID = item.id;

    //            if (item.isSprite) { //if is sprite..

    //                auto& spriteComp = m_registry.Get_Component<Sprite::SpriteComponent>(entID);
    //                auto& transComp = m_registry.Get_Component<TransformComponent>(entID);

    //                spriteComp.Update_ImageSize();

    //                bool isBatched = false;
    //                if (m_registry.Has_Component<TagComponent>(entID))
    //                {
    //                    const auto& tag = m_registry.Get_Component<TagComponent>(entID);
    //                    if (tag.tag == T_BATCH) {
    //                        isBatched = true;
    //                    }
    //                }

    //                if (spriteComp.IsAnimated())
    //                {
    //                    auto& animComp = m_registry.Get_Component<Animator::AnimatorComponent>(entID);
    //                    animComp.UpdateFrame(dt);

    //                    Image img = spriteComp.Get_Image();
    //                    glm::vec4 uv(animComp.u0 / img.widthPixels,
    //                        animComp.v1 / img.heightPixels,
    //                        animComp.u1 / img.widthPixels,
    //                        animComp.v0 / img.heightPixels);

    //                    if (isBatched) {
    //                        renderInstance.SubmitSpriteBatchUV(img, transComp.pos, spriteComp.Get_Scale(), uv, transComp.rot + spriteComp.Get_Rotation());
    //                    }
    //                    else {
    //                        renderInstance.DrawImageSub(img,
    //                            transComp.pos.x, transComp.pos.y,
    //                            spriteComp.Get_Scale().x, spriteComp.Get_Scale().y,
    //                            animComp.u0, animComp.v0, animComp.u1, animComp.v1,
    //                            glm::radians(transComp.rot + spriteComp.Get_Rotation()));
    //                    }
    //                }
    //                else
    //                {
    //                    Image img = spriteComp.Get_Image();
    //                    Vec2 scaledSize = { img.widthPixels * spriteComp.Get_Scale().x, img.heightPixels * spriteComp.Get_Scale().y };

    //                    if (isBatched) {
    //                        renderInstance.SubmitSpriteBatch(img, transComp.pos, scaledSize, transComp.rot + spriteComp.Get_Rotation());
    //                    }
    //                    else {
    //                        renderInstance.DrawImageScaled(img,
    //                            transComp.pos.x, transComp.pos.y,
    //                            spriteComp.Get_Scale().x, spriteComp.Get_Scale().y,
    //                            glm::radians(transComp.rot + spriteComp.Get_Rotation()));
    //                    }
    //                }
    //            }

    //            //shape stuff here (if is shape...)
    //            else {

    //                // Skip entities without a transform (need position/rotation to draw)
    //                if (!m_registry.Has_Component<TransformComponent>(entID)) continue;
    //                TransformComponent& transComp = m_registry.Get_Component<TransformComponent>(entID);
    //                ShapeComponent& shapeComp = m_registry.Get_Component<ShapeComponent>(entID);
    //                Colour3 color = { shapeComp.color.x, shapeComp.color.y, shapeComp.color.z };

    //                if (m_registry.Has_Component<TagComponent>(entID)) {
    //                    TagComponent& tag = m_registry.Get_Component<TagComponent>(entID);

    //                    if (shapeComp.shape == SHAPE_RECTANGLE && tag.tag == T_INSTANCING) { //check entity is using instancing
    //                        //Store matching entities inside InstanceData vector
    //                        InstanceData inst;
    //                        inst.position = transComp.pos;
    //                        inst.scale = transComp.scale;
    //                        inst.rotation = glm::radians(transComp.rot);
    //                        inst.uvRect = { 0,0,1,1 };
    //                        batch.push_back(inst);
    //                        continue;
    //                    }


    //                }

    //                if (shapeComp.shape == SHAPE_RECTANGLE) {
    //                    renderInstance.DrawRect(
    //                        transComp.pos,
    //                        shapeComp.dimensions.x,
    //                        shapeComp.dimensions.y,
    //                        color,
    //                        transComp.rot
    //                    );
    //                }
    //                else if (shapeComp.shape == SHAPE_CIRCLE) {
    //                    float radius = (shapeComp.dimensions.x + shapeComp.dimensions.y) / 4.0f;
    //                    renderInstance.DrawCircle(
    //                        transComp.pos,
    //                        radius,
    //                        color,
    //                        32
    //                    );
    //                }
    //            }
    //        }
    //    };

   // Background (rendered behind game objects)
    if (s_layerEnabled[LAYER_BACKGROUND])
    {
        renderInstance.BeginBatch();
        drawLayer(bg_list);
        renderInstance.EndBatch();
        renderInstance.FlushBatch();
    }

  // Game objects
    if (s_layerEnabled[LAYER_GAMEOBJECT])
    {
        renderInstance.BeginBatch();
        drawLayer(go_list);
        renderInstance.EndBatch();
        renderInstance.FlushBatch();
    }

    // Foreground (rendered above game objects)
    if (s_layerEnabled[LAYER_FOREGROUND])
    {
        renderInstance.BeginBatch();
        drawLayer(fg_list);
        for (auto& [eID, hpBar] : m_registry.Get_ComponentMap<HealthBarComponent>())
        {
            auto& health = m_registry.Get_Component<HealthComponent>(eID);
            auto& transform = m_registry.Get_Component<TransformComponent>(eID);

            if (health.currhp <= 0.f) continue;

            float ratio = health.currhp / health.maxhp;

            Vec2 pos = transform.pos + Vec2{ 0.f, hpBar.yOffset };
            DrawHealthBar(pos, hpBar.width, hpBar.height, ratio);
        }
        renderInstance.EndBatch();
        renderInstance.FlushBatch();
    }

    // UI (rendered last, most in front)
    if (s_layerEnabled[LAYER_UI])
    {
        renderInstance.SetProjectionType(ProjectionType::UI);
        renderInstance.BeginBatch();
        drawLayer(ui_list);
        renderInstance.EndBatch();
        renderInstance.FlushBatch();
    }
    renderer.useCamera = camcheck;

    // Reset projection to actual screen dimensions
    Render::GetInstance().UpdateProjection(false);


    //Draw in the correct order
    /*DrawLayer(backgroundEntities);
      DrawLayer(gameEntities);
      DrawLayer(foregroundEntities);
      DrawLayer(uiEntities);

      renderInstance.EndBatch();
      renderInstance.FlushBatch();*/


    /*//try batching version (old version keep in case need ref)
    renderInstance.BeginBatch(); //added for batch

    for (auto& [entID, spriteComp] : m_registry.Get_ComponentMap<Sprite::SpriteComponent>())
    {
        // Update image size (need to link to event manager probably)
        spriteComp.Update_ImageSize();

        // Check if entity has the batching tag
        bool isBatched = false;
        if (m_registry.Has_Component<TagComponent>(entID))
        {
            const auto& tag = m_registry.Get_Component<TagComponent>(entID);
            if (tag.tag == T_BATCH)
                isBatched = true;
        }

        if (spriteComp.IsAnimated())
        {

            if (!m_registry.Has_Component<Animator::AnimatorComponent>(entID))
            {
                std::cout << "Sprite comp is animated but you tried to get AnimatorComponent! Setting sprite comp anim to false\n";
                spriteComp.Set_Animated(false);
                continue;
            }
            // draw animated logic
            TransformComponent& transComp = m_registry.Get_Component<TransformComponent>(entID);
            if (!m_registry.Has_Component<Animator::AnimatorComponent>(entID))
            {
                std::cout << "RENDERSYSTEM: Sprite comp is animated but you tried to get AnimatorComponent! Setting sprite comp anim to false\n";
                spriteComp.Set_Animated(false);
                continue;
            }
            Animator::AnimatorComponent& animComp = m_registry.Get_Component<Animator::AnimatorComponent>(entID);

            animComp.UpdateFrame(dt);

            //added for batch
            Image img = spriteComp.Get_Image();
            glm::vec4 uv(animComp.u0 / img.widthPixels,
                (animComp.v1 / img.heightPixels),
                animComp.u1 / img.widthPixels,
                (animComp.v0 / img.heightPixels));

            if (isBatched) {
                renderInstance.SubmitSpriteBatchUV(img, transComp.pos, spriteComp.Get_Scale(), uv, transComp.rot + spriteComp.Get_Rotation());
            }
            //renderInstance.SubmitSpriteBatchUV(img, transComp.pos, spriteComp.Get_Scale(), uv, transComp.rot + spriteComp.Get_Rotation());
            //end of added for batch
            
            else {
                renderInstance.DrawImageSub(
                    img,
                    transComp.pos.x, transComp.pos.y,
                    spriteComp.Get_Scale().x, spriteComp.Get_Scale().y,
                    animComp.u0, animComp.v0, animComp.u1, animComp.v1,
                    glm::radians(transComp.rot + spriteComp.Get_Rotation()));
            }
            //renderInstance.DrawImageSub(spriteComp.Get_Image(), transComp.pos.x, transComp.pos.y, spriteComp.Get_Scale().x, spriteComp.Get_Scale().y, animComp.u0, animComp.v0, animComp.u1, animComp.v1, transComp.rot + spriteComp.Get_Rotation());
        }
        else
        {
            TransformComponent& transComp = m_registry.Get_Component<TransformComponent>(entID);

            Image img = spriteComp.Get_Image();
            Vec2 scaledSize = {
                img.widthPixels * spriteComp.Get_Scale().x,
                img.heightPixels * spriteComp.Get_Scale().y
            };

            if (isBatched) {
                renderInstance.SubmitSpriteBatch(
                    img,
                    transComp.pos,
                    scaledSize,
                    transComp.rot + spriteComp.Get_Rotation());
            }
            /*renderInstance.SubmitSpriteBatch(
                img,
                transComp.pos,
                scaledSize,
                transComp.rot + spriteComp.Get_Rotation());*/

            /*else {
                renderInstance.DrawImageScaled(
                    img,
                    transComp.pos.x, transComp.pos.y,
                    spriteComp.Get_Scale().x, spriteComp.Get_Scale().y,
                    glm::radians(transComp.rot + spriteComp.Get_Rotation()));
            }
            //renderInstance.DrawImageScaled(spriteComp.Get_Image(), transComp.pos.x, transComp.pos.y, spriteComp.Get_Scale().x, spriteComp.Get_Scale().y, transComp.rot + spriteComp.Get_Rotation());
        }
    }



    renderInstance.EndBatch();
    renderInstance.FlushBatch();*/


#ifndef NO_IMGUI
    renderInstance.SetProjectionType(ProjectionType::WORLD);

    // Make absolutely sure we're rendering to the scene FBO

    //// ===== TEST: Draw using DrawRect instead of DrawLine =====
    //{
    //    // Try drawing a rectangle which we KNOW works
    //    Vec2 testPos{ 960.f, 540.f };  // Center of screen
    //    Colour3 brightRed{ 1.f, 0.f, 0.f };

    //    renderInstance.DrawRect(testPos, 200.f, 200.f, brightRed);
    //    std::cout << "[RenderSystem] Drew test RECTANGLE at center (960,540)\n";
    //}

    //// Now try the line
    //{
    //    Vec2 testStart{ 0.f, 540.f };
    //    Vec2 testEnd{ 1920.f, 540.f };
    //    Colour3 brightMagenta{ 1.f, 0.f, 1.f };

    //    glLineWidth(10.0f);
    //    renderInstance.DrawLine(testStart, testEnd, brightMagenta);
    //    glLineWidth(1.0f);

    //    std::cout << "[RenderSystem] Drew test LINE\n";
    //}
    //{
    //    Vec2 dummyPos{ -9999.f, -9999.f };  // Way off-screen
    //    Colour3 transparent{ 0.f, 0.f, 0.f };
    //    renderInstance.DrawRect(dummyPos, 1.f, 1.f, transparent, 0.0f, 0.0f); // Fully transparent
    //}
    // Debug drawing for spatial grid
    {
        SpatialGrid* grid = CollisionManager::GetSpatialGrid();
        if (ImGuiSystem::showSpatialGrid) {
            if (grid) {
                DebugDraw::DrawSpatialGrid(grid, renderInstance);
            }
        }
    }

    // Debug drawing for colliders
    if (ImGuiSystem::showColliders) {
        for (auto& [entID, collider] : m_registry.Get_ComponentMap<ColliderComponent>()) {
            if (!m_registry.Has_Component<TransformComponent>(entID)) continue;
            auto& t = m_registry.Get_Component<TransformComponent>(entID);
            std::cout << "[RenderSystem] Drawing AABB for entity " << entID << "\n";
            DebugDraw::DrawAABB(collider.Get_AABB(), renderInstance);
            DebugDraw::DrawHotspot(t.pos, renderInstance);
        }
    }

    // Debug drawing for velocities
    if (ImGuiSystem::showVelocity) {
        for (auto& [entID, physics] : m_registry.Get_ComponentMap<Physics::PhysicsComponent>()) {
            if (!m_registry.Has_Component<TransformComponent>(entID)) continue;
            auto& t = m_registry.Get_Component<TransformComponent>(entID);

            DebugDraw::DrawVelocity(entID, t, renderInstance, dt);
        }
    }

    if (!batch.empty()) {
        //then draw them all at once
        renderInstance.DrawRectInstanced(batch, { 0.2f, 0.8f, 1.0f });
        batch.clear();
    }
    if (ImGuiSystem::guiEnabled) {
        // renderer.EndSceneCapture();
    }
#endif
    //renderer.EndSceneCapture();
    //renderer.PresentScene();
}

void RenderSystem::Shutdown() {
    // clear all image cache here? seems like a good place, if we only store not a lot of images
    Render::GetInstance().ClearAllTextures();
}

namespace CamFollowScript {
    inline void Start(Entity eID) {
        (void)eID;
    }

    inline void Update(Entity eID, float dt) {
        if (!Registry::Get().Has_Component<TransformComponent>(eID)) { return; }
        auto& transComp = Registry::Get().Get_Component<TransformComponent>(eID);
        Camera::GetInstance().FollowTarget(transComp.pos, dt);
    }

    inline void End(Entity eID) {
        (void)eID;
    }
}