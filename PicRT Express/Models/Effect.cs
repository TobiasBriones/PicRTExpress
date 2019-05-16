using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PicRT_Express.Models
{
    class Effect
    {

        public static readonly int HORIZONTAL_RANDOM_LINES = 1;
        public static readonly int PLOT = 2;
        public static readonly int PIXELATED = 3;
        public static readonly int RGB = 4;
        public static readonly int BRIGHTNESS = 5;
        public static readonly int RANDOM_LINES = 6;
        public static readonly int GRAY = 7;
        public static readonly int ROTATE = 8;
        public static readonly int FLIP = 9;
        public static readonly int SEPIA = 10;
        public static readonly int CONVOLUTION = 11;
       /* public static readonly int = ;*/
        private readonly int id;
        private readonly string label;

        public Effect(int id, string label)
        {
            this.id = id;
            this.label = label;
        }

        public override string ToString()
        {
            return label;
        }

        public int getId()
        {
            return id;
        }

    }

}
