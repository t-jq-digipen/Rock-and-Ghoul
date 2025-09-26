// Debug velocity line
/*
-> loop through physics component list
{
	-> if has transform component
	{
		point 1 = transform component position
		point 2 = point 1 + velocity * some scalar shit(from physics component)
		draw line from point 1 to point 2, maybe add arrow tip?(optional!)
	}
}
*/

// Debug hitbox
/*
-> loop through collider list
{
	call render to draw the AABB ^^	need to flesh out render system first xd
}
*/