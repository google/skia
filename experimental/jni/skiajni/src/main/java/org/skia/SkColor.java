package org.skia;


public class SkColor {
  /**
    * Return a color-int from alpha, red, green, blue components.
    * These component values should be \([0..255]\), but there is no
    * range check performed, so if they are out of range, the
    * returned color is undefined.
    * @param alpha Alpha component \([0..255]\) of the color
    * @param red Red component \([0..255]\) of the color
    * @param green Green component \([0..255]\) of the color
    * @param blue Blue component \([0..255]\) of the color
    */
  public static int argb(int alpha, int red, int green, int blue) {
      return (alpha << 24) | (red << 16) | (green << 8) | blue;
  }

}
