package nodes;

import fried.Application;
import fried.input.GamepadButton;
import fried.input.Input;
import fried.input.Key;
import fried.scene.Node;

// Closes the game on Escape or the gamepad's Start. A node like any other: add it to a scene.
@:serializable
class Quit extends Node {
	public function new() {
		super("Quit");
	}

	override function update():Void {
		if (Input.isKeyDown(Key.Escape) || Input.isGamepadButtonDown(GamepadButton.Start)) {
			Application.quit();
		}
	}
}
