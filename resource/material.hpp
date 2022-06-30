/* This file is part of mortar.
 *
 * mortar is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mortar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mortar.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MORTAR_RESOURCE_MATERIAL_H
#define MORTAR_RESOURCE_MATERIAL_H

#include "resource.hpp"
#include "texture.hpp"

namespace Mortar::Resource {
  class Material : public Resource {
    public:
      Material(ResourceHandle handle)
        : Resource { handle },
          red { 0 },
          green { 0 },
          blue { 0 },
          texture { nullptr },
          flags { 0 } {};

    bool isAlphaBlended() const;
    void setIsAlphaBlended(bool isAlphaBlended);

    bool isDynamicallyLit() const;
    void setIsDynamicallyLit(bool isDynamicallyLit);

    const float *getColor() const;
    void setColor(float red, float green, float blue);

    const Texture *getTexture() const;
    void setTexture(Texture *texture);

    private:
      enum MaterialFlags {
        USE_VERTEX_COLOR = 1 << 0,
        ENABLE_ALPHA_BLEND = 1 << 1,
        DYNAMICALLY_LIT = 1 << 2,
      };

      void setFlag(MaterialFlags flag, bool value);

      union {
        struct {
          float red;
          float green;
          float blue;
        };
        float color[3];
      };

      unsigned referenceAlpha;

      Texture *texture;

      unsigned flags;
  };
}

#endif
