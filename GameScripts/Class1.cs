using System;

public class Rotator
{
    public void Update(float dt)
    {
        Console.WriteLine($"[Rotator] Rotating... Δt = {dt}");
    }
}

public class Mover
{
    public void Update(float dt)
    {
        Console.WriteLine($"[Mover] Moving... Δt = {dt}");
    }
}
