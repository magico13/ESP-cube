using System.Collections.Generic;
using Newtonsoft.Json;

namespace Companion
{
    ///<summary>
    /// Breathe animation: Slowly ramp up/down the color over the provided timespan
    ///</summary>
    public class Breathe : IAnimation
    {
        ///<summary>
        /// The type of animation
        ///</summary>
        public string AnimationType => "breathe";
        ///<summary>
        /// The number of times to run through the animation
        ///</summary>
        public int Count { get; set; } = 1;
        ///<summary>
        /// The number of milliseconds for one cyle
        ///</summary>
        public int Length { get; set; } = 1000;
        ///<summary>
        /// The color for the animation
        ///</summary>
        public Color Color { get; set; } = new Color { Blue = 255 };

        ///<summary>
        /// Serialize the animation to send to the Cube
        ///</summary>
        public string ToJson()
        {
            Dictionary<string, object> obj = new Dictionary<string, object>
            {
                ["animation"] = AnimationType,
                ["count"] = Count,
                ["length"] = Length,
                ["color"] = new int[] { Color.Red, Color.Green, Color.Blue }
            };
            return JsonConvert.SerializeObject(obj);
        }
    }
}