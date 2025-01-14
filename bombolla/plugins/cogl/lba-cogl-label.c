/* la Bombolla GObject shell.
 * Copyright (C) 2020 Aleksandr Slobodeniuk
 *
 *   This file is part of bombolla.
 *
 *   Bombolla is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Bombolla is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with bombolla.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "base/lba-base-cogl3d.h"
#include "bombolla/lba-plugin-system.h"
#include "bombolla/lba-log.h"
#include <cogl-pango/cogl-pango.h>

typedef struct _LbaCoglLabel {
  BaseCogl3d parent;

  CoglPangoFontMap *pango_font_map;
  PangoContext *pango_context;
  PangoFontDescription *pango_font_desc;

  PangoLayout *hello_label;
  int hello_label_width;
  int hello_label_height;
  CoglMatrix view;

  CoglColor color;

  gchar *text;
  gchar *font_name;
  guint font_size;
} LbaCoglLabel;

typedef struct _LbaCoglLabelClass {
  BaseCogl3dClass parent;
} LbaCoglLabelClass;

static void
lba_cogl_label_paint (BaseCogl3d * base, CoglFramebuffer * fb,
                      CoglPipeline * pipeline) {
  Base3d *s3d = (Base3d *) base;
  LbaCoglLabel *self = (LbaCoglLabel *) base;

  double x,
    y,
    z;
  int framebuffer_width;
  int framebuffer_height;

  x = s3d->x;
  y = s3d->y;
  z = s3d->z;

  LBA_LOG ("draw (%f, %f, %f)", x, y, z);

  framebuffer_width = cogl_framebuffer_get_width (fb);
  framebuffer_height = cogl_framebuffer_get_height (fb);

  cogl_pango_show_layout (fb, self->hello_label,
                          framebuffer_width * x, framebuffer_height * y,
                          &self->color);
}

static void
lba_cogl_label_update (LbaCoglLabel * self) {
  PangoRectangle hello_label_size;

  if (!self->pango_font_desc || !self->hello_label)
    return;

  pango_font_description_set_family (self->pango_font_desc, self->font_name);
  pango_font_description_set_size (self->pango_font_desc,
                                   self->font_size * PANGO_SCALE);

  pango_layout_set_font_description (self->hello_label, self->pango_font_desc);
  pango_layout_set_text (self->hello_label, self->text, -1);

  pango_layout_get_extents (self->hello_label, NULL, &hello_label_size);
  self->hello_label_width = PANGO_PIXELS (hello_label_size.width);
  self->hello_label_height = PANGO_PIXELS (hello_label_size.height);
}

static void
lba_cogl_label_reopen (BaseCogl3d * base, CoglFramebuffer * fb,
                       CoglPipeline * pipeline, CoglContext * ctx) {
  LbaCoglLabel *self = (LbaCoglLabel *) base;
  int framebuffer_width;
  int framebuffer_height;
  float fovy,
    aspect,
    z_near,
    z_2d,
    z_far;

  LBA_LOG ("reopening");

  framebuffer_width = cogl_framebuffer_get_width (fb);
  framebuffer_height = cogl_framebuffer_get_height (fb);

  /* FIXME: same as CoglCube has */
  fovy = 60;                    /* y-axis field of view */
  aspect = (float)framebuffer_width / (float)framebuffer_height;
  z_near = 0.1;                 /* distance to near clipping plane */
  z_2d = 1000;                  /* position to 2d plane */
  z_far = 2000;                 /* distance to far clipping plane */

  /* So, we reset framebuffer's matrix. This should be done as some
   * common thing, maybe in window */
  cogl_framebuffer_perspective (fb, fovy, aspect, z_near, z_far);

  /* Since the pango renderer emits geometry in pixel/device coordinates
   * and the anti aliasing is implemented with the assumption that the
   * geometry *really* does end up pixel aligned, we setup a modelview
   * matrix so that for geometry in the plane z = 0 we exactly map x
   * coordinates in the range [0,stage_width] and y coordinates in the
   * range [0,stage_height] to the framebuffer extents with (0,0) being
   * the top left.
   *
   * This is roughly what Clutter does for a ClutterStage, but this
   * demonstrates how it is done manually using Cogl.
   */
  cogl_matrix_init_identity (&self->view);
  cogl_matrix_view_2d_in_perspective (&self->view, fovy, aspect, z_near, z_2d,
                                      framebuffer_width, framebuffer_height);
  cogl_framebuffer_set_modelview_matrix (fb, &self->view);

  /* Initialize some convenient constants */
  cogl_color_init_from_4ub (&self->color, 0xff, 0xff, 0xff, 0xff);

  /* Setup a Pango font map and context */

  self->pango_font_map = COGL_PANGO_FONT_MAP (cogl_pango_font_map_new ());

  cogl_pango_font_map_set_use_mipmapping (self->pango_font_map, TRUE);

  self->pango_context = cogl_pango_font_map_create_context (self->pango_font_map);

  /* FIXME: free before */
  self->pango_font_desc = pango_font_description_new ();
  self->hello_label = pango_layout_new (self->pango_context);

  lba_cogl_label_update (self);
}

static void
lba_cogl_label_init (LbaCoglLabel * self) {
}

typedef enum {
  PROP_TEXT = 1,
  PROP_FONT_NAME,
  PROP_FONT_SIZE,
  /* TODO: color */
  N_PROPERTIES
} LbaCoglLabelProperty;

static void
lba_cogl_label_set_property (GObject * object,
                             guint property_id, const GValue * value,
                             GParamSpec * pspec) {
  LbaCoglLabel *self = (LbaCoglLabel *) object;

  switch ((LbaCoglLabelProperty) property_id) {
  case PROP_TEXT:
    g_free (self->text);
    self->text = g_value_dup_string (value);

    lba_cogl_label_update (self);
    break;

  case PROP_FONT_NAME:
    g_free (self->font_name);
    self->font_name = g_value_dup_string (value);

    lba_cogl_label_update (self);
    break;

  case PROP_FONT_SIZE:
    self->font_size = g_value_get_uint (value);

    lba_cogl_label_update (self);
    break;

  default:
    /* We don't have any other property... */
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
lba_cogl_label_get_property (GObject * object,
                             guint property_id, GValue * value, GParamSpec * pspec) {
  LbaCoglLabel *self = (LbaCoglLabel *) object;

  switch ((LbaCoglLabelProperty) property_id) {
  case PROP_TEXT:
    g_value_set_string (value, self->text);
    break;

  case PROP_FONT_NAME:
    g_value_set_string (value, self->font_name);
    break;

  case PROP_FONT_SIZE:
    g_value_set_uint (value, self->font_size);
    break;

  default:
    /* We don't have any other property... */
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
lba_cogl_label_class_init (LbaCoglLabelClass * klass) {
  GObjectClass *gobj_class = G_OBJECT_CLASS (klass);
  BaseCogl3dClass *base_cogl3d_class = (BaseCogl3dClass *) klass;

  base_cogl3d_class->paint = lba_cogl_label_paint;
  base_cogl3d_class->reopen = lba_cogl_label_reopen;

  gobj_class->set_property = lba_cogl_label_set_property;
  gobj_class->get_property = lba_cogl_label_get_property;

  g_object_class_install_property (gobj_class,
                                   PROP_TEXT,
                                   g_param_spec_string ("text",
                                                        "text",
                                                        "text",
                                                        "Hello from lbaCoglLabel",
                                                        G_PARAM_STATIC_STRINGS |
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT));

  g_object_class_install_property (gobj_class,
                                   PROP_FONT_NAME,
                                   g_param_spec_string ("font-name",
                                                        "font-name",
                                                        "font-name",
                                                        "Sans",
                                                        G_PARAM_STATIC_STRINGS |
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT));

  g_object_class_install_property (gobj_class, PROP_FONT_SIZE,
                                   g_param_spec_uint ("font-size",
                                                      "font-size", "font-size",
                                                      0 /* minimum value */ ,
                                                      G_MAXUINT /* maximum value */ ,
                                                      30 /* default value */ ,
                                                      G_PARAM_STATIC_STRINGS |
                                                      G_PARAM_READWRITE |
                                                      G_PARAM_CONSTRUCT));
}

G_DEFINE_TYPE (LbaCoglLabel, lba_cogl_label, G_TYPE_BASE_COGL3D)
/* Export plugin */
    BOMBOLLA_PLUGIN_SYSTEM_PROVIDE_GTYPE (lba_cogl_label);
