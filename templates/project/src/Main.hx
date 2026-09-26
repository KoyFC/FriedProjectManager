import fried.Application;
import fried.Project;
import fried.Window;
import fried.graphics.Texture;
import fried.input.Input;
import fried.input.Key;
import fried.io.Assets;

class Main {
	static inline var SQUARE_SIZE = 160;

	public static function main():Void {
		Application.init();

		var window = new Window(Project.windowTitle(), 960, 540);
		window.onClose = function() {
			Application.quit();
		};

		var renderer = Application.createRenderer(window);
		renderer.setDrawColor(24, 24, 32);

		var square = Texture.load(renderer, Assets.game("white.png"));

		Application.run(function() {
			if (Input.isKeyDown(Key.Escape)) {
				Application.quit();
			}

			renderer.drawTexture(square, Std.int((renderer.width - SQUARE_SIZE) / 2), Std.int((renderer.height - SQUARE_SIZE) / 2), SQUARE_SIZE,
				SQUARE_SIZE);
		});

		square.destroy();
		Application.destroyRenderer();
		window.destroy();
		Application.shutdown();
	}
}
