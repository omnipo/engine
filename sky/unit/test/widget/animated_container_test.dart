import 'package:flutter/animation.dart';
import 'package:flutter/rendering.dart';
import 'package:flutter/widgets.dart';
import 'package:test/test.dart';

import 'widget_tester.dart';

void main() {
  test('AnimatedContainer control test', () {
    testWidgets((WidgetTester tester) {
      GlobalKey key = new GlobalKey();

      BoxDecoration decorationA = new BoxDecoration(
        backgroundColor: new Color(0xFF00FF00)
      );

      BoxDecoration decorationB = new BoxDecoration(
        backgroundColor: new Color(0xFF0000FF)
      );

      tester.pumpWidget(
        new AnimatedContainer(
          key: key,
          duration: const Duration(milliseconds: 200),
          decoration: decorationA
        )
      );

      RenderDecoratedBox box = key.currentState.context.findRenderObject();
      expect(box.decoration.backgroundColor, equals(decorationA.backgroundColor));

      tester.pumpWidget(
        new AnimatedContainer(
          key: key,
          duration: const Duration(milliseconds: 200),
          decoration: decorationB
        )
      );

      expect(key.currentState.context.findRenderObject(), equals(box));
      expect(box.decoration.backgroundColor, equals(decorationA.backgroundColor));

      tester.pump(const Duration(seconds: 1));

      expect(box.decoration.backgroundColor, equals(decorationB.backgroundColor));

    });
  });

  test('AnimatedContainer overanimate test', () {
    testWidgets((WidgetTester tester) {
      tester.pumpWidget(
        new AnimatedContainer(
          duration: const Duration(milliseconds: 200),
          decoration: new BoxDecoration(
            backgroundColor: new Color(0xFF00FF00)
          )
        )
      );
      expect(scheduler.transientCallbackCount, 0);
      tester.pump(new Duration(seconds: 1));
      expect(scheduler.transientCallbackCount, 0);
      tester.pumpWidget(
        new AnimatedContainer(
          duration: const Duration(milliseconds: 200),
          decoration: new BoxDecoration(
            backgroundColor: new Color(0xFF00FF00)
          )
        )
      );
      expect(scheduler.transientCallbackCount, 0);
      tester.pump(new Duration(seconds: 1));
      expect(scheduler.transientCallbackCount, 0);
      tester.pumpWidget(
        new AnimatedContainer(
          duration: const Duration(milliseconds: 200),
          decoration: new BoxDecoration(
            backgroundColor: new Color(0xFF0000FF)
          )
        )
      );
      expect(scheduler.transientCallbackCount, 1); // this is the only time an animation should have started!
      tester.pump(new Duration(seconds: 1));
      expect(scheduler.transientCallbackCount, 0);
      tester.pumpWidget(
        new AnimatedContainer(
          duration: const Duration(milliseconds: 200),
          decoration: new BoxDecoration(
            backgroundColor: new Color(0xFF0000FF)
          )
        )
      );
      expect(scheduler.transientCallbackCount, 0);
    });
  });
}
