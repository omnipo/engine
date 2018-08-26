import 'package:flutter/animation.dart';
import 'package:flutter/gestures.dart';
import 'package:flutter/rendering.dart';
import 'package:flutter/widgets.dart';
import 'package:quiver/testing/async.dart';
import 'package:quiver/time.dart';

import '../engine/mock_events.dart';

class RootComponent extends StatefulComponent {
  RootComponentState createState() => new RootComponentState();
}

class RootComponentState extends State<RootComponent> {
  Widget _child = new DecoratedBox(decoration: new BoxDecoration());
  Widget get child => _child;
  void set child(Widget value) {
    if (value != _child) {
      setState(() {
        _child = value;
      });
    }
  }
  Widget build(BuildContext context) => child;
}

typedef Point SizeToPointFunction(Size size);

class WidgetTester {
  WidgetTester._(FakeAsync async)
    : async = async,
      clock = async.getClock(new DateTime.utc(2015, 1, 1));

  final FakeAsync async;
  final Clock clock;

  void pumpWidget(Widget widget, [ Duration duration ]) {
    runApp(widget);
    pump(duration);
  }

  void pump([ Duration duration ]) {
    if (duration != null)
      async.elapse(duration);
    scheduler.beginFrame(new Duration(milliseconds: clock.now().millisecondsSinceEpoch));
    async.flushMicrotasks();
  }

  List<Layer> _layers(Layer layer) {
    List<Layer> result = <Layer>[layer];
    if (layer is ContainerLayer) {
      ContainerLayer root = layer;
      Layer child = root.firstChild;
      while(child != null) {
        result.addAll(_layers(child));
        child = child.nextSibling;
      }
    }
    return result;
  }
  List<Layer> get layers => _layers(FlutterBinding.instance.renderView.layer);


  void walkElements(ElementVisitor visitor) {
    void walk(Element element) {
      visitor(element);
      element.visitChildren(walk);
    }
    WidgetFlutterBinding.instance.renderViewElement.visitChildren(walk);
  }

  Element findElement(bool predicate(Element element)) {
    try {
      walkElements((Element element) {
        if (predicate(element))
          throw element;
      });
    } on Element catch (e) {
      return e;
    }
    return null;
  }

  Element findElementByKey(Key key) {
    return findElement((Element element) => element.widget.key == key);
  }

  Element findText(String text) {
    return findElement((Element element) {
      return element.widget is Text && element.widget.data == text;
    });
  }

  State findStateOfType(Type type) {
    StatefulComponentElement element = findElement((Element element) {
      return element is StatefulComponentElement && element.state.runtimeType == type;
    });
    return element?.state;
  }

  State findStateByConfig(Widget config) {
    StatefulComponentElement element = findElement((Element element) {
      return element is StatefulComponentElement && element.state.config == config;
    });
    return element?.state;
  }

  Point getCenter(Element element) {
    return _getElementPoint(element, (Size size) => size.center(Point.origin));
  }

  Point getTopLeft(Element element) {
    return _getElementPoint(element, (_) => Point.origin);
  }

  Point getTopRight(Element element) {
    return _getElementPoint(element, (Size size) => size.topRight(Point.origin));
  }

  Point getBottomLeft(Element element) {
    return _getElementPoint(element, (Size size) => size.bottomLeft(Point.origin));
  }

  Point getBottomRight(Element element) {
    return _getElementPoint(element, (Size size) => size.bottomRight(Point.origin));
  }

  Point _getElementPoint(Element element, SizeToPointFunction sizeToPoint) {
    assert(element != null);
    RenderBox box = element.renderObject as RenderBox;
    assert(box != null);
    return box.localToGlobal(sizeToPoint(box.size));
  }


  void tap(Element element, { int pointer: 1 }) {
    tapAt(getCenter(element), pointer: pointer);
  }

  void tapAt(Point location, { int pointer: 1 }) {
    HitTestResult result = _hitTest(location);
    TestPointer p = new TestPointer(pointer);
    _dispatchEvent(p.down(location), result);
    _dispatchEvent(p.up(), result);
  }

  void scroll(Element element, Offset offset, { int pointer: 1 }) {
    scrollAt(getCenter(element), offset, pointer: pointer);
  }

  void scrollAt(Point startLocation, Offset offset, { int pointer: 1 }) {
    Point endLocation = startLocation + offset;
    TestPointer p = new TestPointer(pointer);
    // Events for the entire press-drag-release gesture are dispatched
    // to the widgets "hit" by the pointer down event.
    HitTestResult result = _hitTest(startLocation);
    _dispatchEvent(p.down(startLocation), result);
    _dispatchEvent(p.move(endLocation), result);
    _dispatchEvent(p.up(), result);
  }

  void dispatchEvent(InputEvent event, Point location) {
    _dispatchEvent(event, _hitTest(location));
  }

  HitTestResult _hitTest(Point location) => WidgetFlutterBinding.instance.hitTest(location);

  void _dispatchEvent(InputEvent event, HitTestResult result) {
    WidgetFlutterBinding.instance.dispatchEvent(event, result);
  }

}

void testWidgets(callback(WidgetTester tester)) {
  new FakeAsync().run((FakeAsync async) {
    callback(new WidgetTester._(async));
  });
}
