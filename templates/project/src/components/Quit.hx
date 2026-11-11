package components;

import fried.Application;
import fried.input.GamepadButton;
import fried.input.Input;
import fried.input.Key;
import fried.scene.Component;

// Closes the game on Escape or the gamepad's Start. A component like any other: add it to an object in a scene.
@:serializable
class Quit extends Component {
	override function update():Void {
		if (Input.isKeyDown(Key.Escape) || Input.isGamepadButtonDown(GamepadButton.Start)) {
			Application.quit();
		}
	}
}
