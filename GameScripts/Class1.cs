using System;
using System.Runtime.InteropServices;

public static class ECS
{
    [DllImport("team_aviddivas.exe", EntryPoint = "CS_GetPosition")]
    public static extern void GetPosition(uint entity, out float x, out float y);

    [DllImport("team_aviddivas.exe", EntryPoint = "CS_SetPosition")]
    public static extern void SetPosition(uint entity, float x, float y);
}
public static class InputBridge
{
    [DllImport("team_aviddivas.exe", EntryPoint = "CS_IsKeyPressed")]
    public static extern bool IsKeyPressed(int key);
}
public class Rotator
{
    public uint EntityID;

    public void Update(float dt)
    {
        ECS.GetPosition(EntityID, out float x, out float y);
        x += 100f * dt; // move right
        ECS.SetPosition(EntityID, x, y);
        Console.WriteLine($"[Rotator] Entity {EntityID} pos=({x:F1},{y:F1})");
    }
}

public class Mover
{
    public uint EntityID;

    public void Update(float dt)
    {
        ECS.GetPosition(EntityID, out float x, out float y);
        y += 50f * dt; // move upward
        ECS.SetPosition(EntityID, x, y);
        Console.WriteLine($"[Mover] Entity {EntityID} pos=({x:F1},{y:F1})");
    }
}
