using System.Threading.Tasks;
using System.Net.Http;
using Newtonsoft.Json;

namespace Companion
{
    ///<summary>
    /// Class for interacting with a specific Cube instance
    ///</summary>
    public class Cube
    {
        private const string COLOR_PATH = "/color";
        private const string ANIMATE_PATH = "/animate";
        private const string TAP_PATH = "/tap";

        private string _url;
        private static HttpClient _client = new HttpClient();

        ///<summary>
        /// Connect to a Cube at the specified url, eg. http://esp-cube.local
        ///</summary>
        public Cube(string url)
        {
            _url = url;
        }

        ///<summary>
        /// Returns the current "base" color of the cube
        ///</summary>
        public Color GetColor()
        {
            string result = GetAsync(COLOR_PATH).Result;
            if (!string.IsNullOrEmpty(result))
            {
                return JsonConvert.DeserializeObject<Color>(result);
            }
            return new Color();
        }

        ///<summary>
        /// Sets the base color of the cube
        ///</summary>
        public void SetColor(Color color)
        {
            string encoded = JsonConvert.SerializeObject(color).ToLower();
            PostAsync(COLOR_PATH, encoded).Wait();
        }

        ///<summary>
        /// Triggers the specified animation
        ///</summary>
        public void Animate(IAnimation animation)
        {
            PostAsync(ANIMATE_PATH, animation.ToJson()).Wait();
        }

        ///<summary>
        /// Sets the tap action to the specified animation
        ///</summary>
        public void SetTap(IAnimation animation)
        {
            PostAsync(TAP_PATH, animation.ToJson()).Wait();
        }

        ///<summary>
        /// GET from the cube
        ///</summary>
        private async Task<string> GetAsync(string path)
        {
            HttpResponseMessage response = await _client.GetAsync(_url+path);
            response.EnsureSuccessStatusCode();
            return await response.Content.ReadAsStringAsync();
        }

        ///<summary>
        /// POST to the cube
        ///</summary>
        private async Task<string> PostAsync(string path, string data)
        {
            HttpContent content = new StringContent(data);
            var response = await _client.PostAsync(_url+path, content);
            response.EnsureSuccessStatusCode();
            return await response.Content.ReadAsStringAsync();
        }
    }
}
