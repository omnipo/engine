import 'package:flutter/rendering.dart';
import 'package:flutter/widgets.dart';
import 'package:test/test.dart';

import 'widget_tester.dart';

void main() {
  test('Can hit test flex children of stacks', () {
    testWidgets((WidgetTester tester) {
      bool didReceiveTap = false;
      tester.pumpWidget(
        new Container(
          decoration: const BoxDecoration(
            backgroundColor: const Color(0xFF00FF00)
          ),
          child: new Stack(<Widget>[
            new Positioned(
              top: 10.0,
              left: 10.0,
              child: new Column(<Widget>[
                new GestureDetector(
                  onTap: () {
                    didReceiveTap = true;
                  },
                  child: new Container(
                    decoration: const BoxDecoration(
                      backgroundColor: const Color(0xFF0000FF)
                    ),
                    width: 100.0,
                    height: 100.0,
                    child: new Center(
                      child: new Text('X')
                    )
                  )
                )
              ])
            )
          ])
        )
      );

      tester.tap(tester.findText('X'));
      expect(didReceiveTap, isTrue);
    });
  });

  test('Row, Column and FlexJustifyContent.collapse', () {
    final Key flexKey = new Key('flexKey');

    // Row without justifyContent: FlexJustifyContent.collapse
    testWidgets((WidgetTester tester) {
      tester.pumpWidget(new Center(
        child: new Row([
          new Container(width: 10.0, height: 100.0),
          new Container(width: 30.0, height: 100.0)
        ],
          key: flexKey
        )
      ));
      RenderBox renderBox = tester.findElementByKey(flexKey).renderObject;
      expect(renderBox.size.width, equals(800.0));
      expect(renderBox.size.height, equals(100.0));

      // Row with justifyContent: FlexJustifyContent.collapse
      tester.pumpWidget(new Center(
        child: new Row([
          new Container(width: 10.0, height: 100.0),
          new Container(width: 30.0, height: 100.0)
        ],
          key: flexKey,
          justifyContent: FlexJustifyContent.collapse
        )
      ));
      renderBox = tester.findElementByKey(flexKey).renderObject;
      expect(renderBox.size.width, equals(40.0));
      expect(renderBox.size.height, equals(100.0));
    });

    // Column without justifyContent: FlexJustifyContent.collapse
    testWidgets((WidgetTester tester) {
      tester.pumpWidget(new Center(
        child: new Column([
          new Container(width: 100.0, height: 100.0),
          new Container(width: 100.0, height: 150.0)
        ],
          key: flexKey
        )
      ));
      RenderBox renderBox = tester.findElementByKey(flexKey).renderObject;
      expect(renderBox.size.width, equals(100.0));
      expect(renderBox.size.height, equals(600.0));

      // Column with justifyContent: FlexJustifyContent.collapse
      tester.pumpWidget(new Center(
        child: new Column([
          new Container(width: 100.0, height: 100.0),
          new Container(width: 100.0, height: 150.0)
        ],
          key: flexKey,
          justifyContent: FlexJustifyContent.collapse
        )
      ));
      renderBox = tester.findElementByKey(flexKey).renderObject;
      expect(renderBox.size.width, equals(100.0));
      expect(renderBox.size.height, equals(250.0));
    });
  });
}
