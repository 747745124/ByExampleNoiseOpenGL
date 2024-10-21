#include "./SynthBuffer.hpp"
// void SynthBuffer::aobuffer_init(int size_x, int size_y)
// {
//     unsigned SCR_WIDTH = size_x;
//     unsigned SCR_HEIGHT = size_y;

//     glGenFramebuffers(1, &aobuffer);
//     glBindFramebuffer(GL_FRAMEBUFFER, aobuffer);

//     glGenTextures(1, &gtao_color_text);
//     glBindTexture(GL_TEXTURE_2D, gtao_color_text);
//     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//     glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gtao_color_text, 0);

//     glDrawBuffer(GL_COLOR_ATTACHMENT0);

//     if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
//         std::cout << "Framebuffer not complete!" << std::endl;

//     glBindFramebuffer(GL_FRAMEBUFFER, 0);
// }

// void GTAOBuffer::cleanUp()
// {
//     glDeleteFramebuffers(1, &aobuffer);
//     glDeleteTextures(1, &gtao_color_text);
// }

// void GTAOBuffer::bindToWrite()
// {
//     glBindFramebuffer(GL_FRAMEBUFFER, aobuffer);
// }

// void GTAOBuffer::bindToRead()
// {
//     glBindFramebuffer(GL_READ_FRAMEBUFFER, aobuffer);
// }

// void GTAOBuffer::bindTextureByIndex(int index)
// {
//     switch (index)
//     {
//     case 0:
//         glBindTexture(GL_TEXTURE_2D, gtao_color_text);
//         break;
//     default:
//         glBindTexture(GL_TEXTURE_2D, gtao_color_text);
//         break;
//     }
// };
