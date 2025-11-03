using System;
using System.Runtime.InteropServices;

public static class ECS
{
    [DllImport("team_aviddivas.exe", EntryPoint = "CS_GetPosition")]
    public static extern void GetPosition(uint entity, out float x, out float y);

    [DllImport("team_aviddivas.exe", EntryPoint = "CS_SetPosition")]
    public static extern void SetPosition(uint entity, float x, float y);

    [DllImport("team_aviddivas.exe", EntryPoint = "CS_GetRotation")]
    public static extern float GetRotation(uint entity);

    [DllImport("team_aviddivas.exe", EntryPoint = "CS_SetRotation")]
    public static extern void SetRotation(uint entity, float rot);
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
        float currentRotation = ECS.GetRotation(EntityID);
        currentRotation += 90f * dt;  // rotate 90 degrees per second
        ECS.SetRotation(EntityID, currentRotation);

        // Optional: Debug print
        Console.WriteLine($"[Rotator] Entity {EntityID} rotation={currentRotation:F1}");
    }
}

public class Mover
{
    public uint EntityID;
    private float time;
    public void Update(float dt)
    {
        ECS.GetPosition(EntityID, out float x, out float y);

        // 2. Move up & down with a sine wave
        time += dt;
        float amplitude = 100.0f;   // how far up/down
        float speed = 2.0f;         // how fast it moves
        float newY = 500.0f + (float)Math.Sin(time * speed) * amplitude;

        // 3. Write new position
        ECS.SetPosition(EntityID, x, newY);
    }
}
