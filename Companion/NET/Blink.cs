using System.Collections.Generic;
using Newtonsoft.Json;

namespace Companion
{
    ///<summary>
    /// Blink animation: Quickly change the color between two colors
    ///</summary>
    public class Blink : IAnimation
    {
        ///<summary>
        /// The type of animation
        ///</summary>
        public string AnimationType => "blink";
        ///<summary>
        /// The number of times to run through the animation
        ///</summary>
        public int Count { get; set; } = 1;
        ///<summary>
        /// Number of milliseconds to display each color
        ///</summary>
        public int Wait { get; set; } = 100;
        ///<summary>
        /// The first color to display
        ///</summary>
        public Color Color1 { get; set; } = new Color {Blue = 255};
        ///<summary>
        /// The second color to display. Use 0, 0, 0 for off.
        ///</summary>
        public Color Color2 { get; set; } = new Color();

        ///<summary>
        /// Serialize the animation to send to the Cube
        ///</summary>
        public string ToJson()
        {
            Dictionary<string, object> obj = new Dictionary<string, object>
            {
                ["animation"] = AnimationType,
                ["count"] = Count,
                ["wait"] = Wait,
                ["color"] = new int[] { Color1.Red, Color1.Green, Color1.Blue },
                ["color2"] = new int[] { Color2.Red, Color2.Green, Color2.Blue }
            };
            return JsonConvert.SerializeObject(obj);
        }
    }
}