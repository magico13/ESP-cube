namespace Companion
{
    public interface IAnimation
    {
        string AnimationType { get; }
        int Count { get; set; }

        string ToJson();
    }
}