import 'package:flutter/rendering.dart';
import 'package:flutter/widgets.dart';
import 'package:test/test.dart';

import 'widget_tester.dart';

void main() {
  test('FractionallySizedBox', () {
    testWidgets((WidgetTester tester) {
      Size detectedSize;
      GlobalKey inner = new GlobalKey();
      tester.pumpWidget(new OverflowBox(
        minWidth: 0.0,
        maxWidth: 100.0,
        minHeight: 0.0,
        maxHeight: 100.0,
        child: new Center(
          child: new FractionallySizedBox(
            width: 0.5,
            height: 0.25,
            child: new SizeObserver(
              onSizeChanged: (Size size) {
                detectedSize = size;
              },
              child: new Container(
                key: inner
              )
            )
          )
        )
      ));
      expect(detectedSize, equals(const Size(50.0, 25.0)));
      RenderBox box = inner.currentContext.findRenderObject();
      expect(box.localToGlobal(Point.origin), equals(const Point(25.0, 37.5)));
    });
  });
}
