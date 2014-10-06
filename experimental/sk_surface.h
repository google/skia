

typedef uint32_t sk_color_t;

sk_color_t sk_color_set_argb(uint8_t a, uint8_t r, uint8_t g, uint8_t b);
uint8_t sk_color_get_a(sk_color_t);

typedef enum {
    UNKNOWN_SK_COLORTYPE,
    RGBA_8888_SK_COLORTYPE,
    BGRA_8888_SK_COLORTYPE,
    ALPHA_8_SK_COLORTYPE,
} sk_colortype_t;

typedef struct sk_imageinfo_t {
    int32_t         width;
    int32_t         height;
    sk_colortype_t  colorType;
    sk_alphatype_t  colorType;
};

typedef struct sk_rect_t {
    float   left;
    float   top;
    float   right;
    float   bottom;
};

typedef struct sk_matrix_t {
    float mat[9];
};

void sk_matrix_set_identity(sk_matrix_t*);

typedef struct sk_path_t;

sk_path_t* sk_path_new();
void sk_path_move_to(sk_path*, float x, float y);
void sk_path_line_to(sk_path*, float x, float y);
void sk_path_quad_to(sk_path*, float x0, float y1, float x1, float y1);
void sk_path_get_bounds(const sk_path_t*, sk_rect_t*);

typedef struct sk_paint_t;

sk_paint_t* sk_paint_new();
bool sk_paint_is_antialias(sk_paint_t*);
void sk_paint_set_antialias(sk_paint_t*, bool);
sk_color_t sk_paint_get_color(const sk_paint_t*);
void sk_paint_set_color(sk_paint_t*, sk_color_t);

typedef struct sk_canvas_t;

void sk_canvas_save(sk_canvas_t*);
void sk_canvas_save_layer(sk_canvas_t*, const sk_rect_t*, const sk_paint_t*);
void sk_canvas_restore(sk_canvas_t*);

void sk_canvas_translate(sk_canvas_t*, sk_scalar_t dx, sk_scalar_t dy);
void sk_canvas_scale(sk_canvas_t*, sk_scalar_t sx, sk_scalar_t sy);
void sk_canvas_concat(sk_canvas_t*, const sk_matrix_t*);

void sk_canvas_draw_paint(sk_canvas_t*, const sk_paint_t*);
void sk_canvas_draw_rect(sk_canvas_t*, const sk_rect_t*, const sk_paint_t*);
void sk_canvas_draw_oval(sk_canvas_t*, const sk_rect_t*, const sk_paint_t*);
void sk_canvas_draw_path(sk_canvas_t*, const sk_path_t*, const sk_paint_t*);
void sk_canvas_draw_image(sk_canvas_t*, const sk_image_t*, float x, float y, const sk_paint_t*);

typedef struct sk_image_t;

sk_image_t* sk_image_new_raster_copy(const sk_image_info_t*, const void* pixels, size_t rowBytes);

int sk_image_get_width(const sk_image_t*);
int sk_image_get_height(const sk_image_t*);
uint32_t sk_image_get_unique_id(const sk_image_t*);

typedef struct sk_surface_t;

sk_surface_t* sk_surface_new_raster(const sk_image_info_t*)
sk_surface_t* sk_surface_new_raster_direct(const sk_image_info_t*, void* pixels, size_t rowBytes);
void sk_surface_delete(sk_surface_t*);

sk_canvas_t* sk_surface_get_canvas(sk_surface_t*);
sk_image_t* sk_surface_new_image_snapshot(sk_surface_t*);

