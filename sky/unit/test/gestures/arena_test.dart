import 'package:flutter/gestures.dart';
import 'package:test/test.dart';

typedef void GestureArenaCallback(Object key);

class TestGestureArenaMember extends GestureArenaMember {
  TestGestureArenaMember({ this.onAcceptGesture, this.onRejectGesture });

  final GestureArenaCallback onAcceptGesture;
  final GestureArenaCallback onRejectGesture;

  void acceptGesture(Object key) {
    onAcceptGesture(key);
  }

  void rejectGesture(Object key) {
    onRejectGesture(key);
  }
}

void main() {
  test('Should win by accepting', () {
    GestureArena arena = new GestureArena();

    int primaryKey = 4;
    bool firstAcceptRan = false;
    bool firstRejectRan = false;
    bool secondAcceptRan = false;
    bool secondRejectRan = false;

    TestGestureArenaMember first = new TestGestureArenaMember(
      onAcceptGesture: (int key) {
        expect(key, equals(primaryKey));
        firstAcceptRan = true;
      },
      onRejectGesture: (int key) {
        expect(key, equals(primaryKey));
        firstRejectRan = true;
      }
    );

    TestGestureArenaMember second = new TestGestureArenaMember(
      onAcceptGesture: (int key) {
        expect(key, equals(primaryKey));
        secondAcceptRan = true;
      },
      onRejectGesture: (int key) {
        expect(key, equals(primaryKey));
        secondRejectRan = true;
      }
    );

    GestureArenaEntry firstEntry = arena.add(primaryKey, first);
    arena.add(primaryKey, second);
    arena.close(primaryKey);

    expect(firstAcceptRan, isFalse);
    expect(firstRejectRan, isFalse);
    expect(secondAcceptRan, isFalse);
    expect(secondRejectRan, isFalse);

    firstEntry.resolve(GestureDisposition.accepted);

    expect(firstAcceptRan, isTrue);
    expect(firstRejectRan, isFalse);
    expect(secondAcceptRan, isFalse);
    expect(secondRejectRan, isTrue);
  });

  test('Should win by sweep', () {
    GestureArena arena = new GestureArena();

    int primaryKey = 4;
    bool firstAcceptRan = false;
    bool firstRejectRan = false;
    bool secondAcceptRan = false;
    bool secondRejectRan = false;

    TestGestureArenaMember first = new TestGestureArenaMember(
      onAcceptGesture: (int key) {
        expect(key, equals(primaryKey));
        firstAcceptRan = true;
      },
      onRejectGesture: (int key) {
        expect(key, equals(primaryKey));
        firstRejectRan = true;
      }
    );

    TestGestureArenaMember second = new TestGestureArenaMember(
      onAcceptGesture: (int key) {
        expect(key, equals(primaryKey));
        secondAcceptRan = true;
      },
      onRejectGesture: (int key) {
        expect(key, equals(primaryKey));
        secondRejectRan = true;
      }
    );

    arena.add(primaryKey, first);
    arena.add(primaryKey, second);
    arena.close(primaryKey);

    expect(firstAcceptRan, isFalse);
    expect(firstRejectRan, isFalse);
    expect(secondAcceptRan, isFalse);
    expect(secondRejectRan, isFalse);

    arena.sweep(primaryKey);

    expect(firstAcceptRan, isTrue);
    expect(firstRejectRan, isFalse);
    expect(secondAcceptRan, isFalse);
    expect(secondRejectRan, isTrue);
  });

  test('Should win on release after hold sweep release', () {
    GestureArena arena = new GestureArena();

    int primaryKey = 4;
    bool firstAcceptRan = false;
    bool firstRejectRan = false;
    bool secondAcceptRan = false;
    bool secondRejectRan = false;

    TestGestureArenaMember first = new TestGestureArenaMember(
      onAcceptGesture: (int key) {
        expect(key, equals(primaryKey));
        firstAcceptRan = true;
      },
      onRejectGesture: (int key) {
        expect(key, equals(primaryKey));
        firstRejectRan = true;
      }
    );

    TestGestureArenaMember second = new TestGestureArenaMember(
      onAcceptGesture: (int key) {
        expect(key, equals(primaryKey));
        secondAcceptRan = true;
      },
      onRejectGesture: (int key) {
        expect(key, equals(primaryKey));
        secondRejectRan = true;
      }
    );

    arena.add(primaryKey, first);
    arena.add(primaryKey, second);
    arena.close(primaryKey);

    expect(firstAcceptRan, isFalse);
    expect(firstRejectRan, isFalse);
    expect(secondAcceptRan, isFalse);
    expect(secondRejectRan, isFalse);

    arena.hold(primaryKey);

    expect(firstAcceptRan, isFalse);
    expect(firstRejectRan, isFalse);
    expect(secondAcceptRan, isFalse);
    expect(secondRejectRan, isFalse);

    arena.sweep(primaryKey);

    expect(firstAcceptRan, isFalse);
    expect(firstRejectRan, isFalse);
    expect(secondAcceptRan, isFalse);
    expect(secondRejectRan, isFalse);

    arena.release(primaryKey);

    expect(firstAcceptRan, isTrue);
    expect(firstRejectRan, isFalse);
    expect(secondAcceptRan, isFalse);
    expect(secondRejectRan, isTrue);
  });

  test('Should win on sweep after hold release sweep', () {
    GestureArena arena = new GestureArena();

    int primaryKey = 4;
    bool firstAcceptRan = false;
    bool firstRejectRan = false;
    bool secondAcceptRan = false;
    bool secondRejectRan = false;

    TestGestureArenaMember first = new TestGestureArenaMember(
      onAcceptGesture: (int key) {
        expect(key, equals(primaryKey));
        firstAcceptRan = true;
      },
      onRejectGesture: (int key) {
        expect(key, equals(primaryKey));
        firstRejectRan = true;
      }
    );

    TestGestureArenaMember second = new TestGestureArenaMember(
      onAcceptGesture: (int key) {
        expect(key, equals(primaryKey));
        secondAcceptRan = true;
      },
      onRejectGesture: (int key) {
        expect(key, equals(primaryKey));
        secondRejectRan = true;
      }
    );

    arena.add(primaryKey, first);
    arena.add(primaryKey, second);
    arena.close(primaryKey);

    expect(firstAcceptRan, isFalse);
    expect(firstRejectRan, isFalse);
    expect(secondAcceptRan, isFalse);
    expect(secondRejectRan, isFalse);

    arena.hold(primaryKey);

    expect(firstAcceptRan, isFalse);
    expect(firstRejectRan, isFalse);
    expect(secondAcceptRan, isFalse);
    expect(secondRejectRan, isFalse);

    arena.release(primaryKey);

    expect(firstAcceptRan, isFalse);
    expect(firstRejectRan, isFalse);
    expect(secondAcceptRan, isFalse);
    expect(secondRejectRan, isFalse);

    arena.sweep(primaryKey);

    expect(firstAcceptRan, isTrue);
    expect(firstRejectRan, isFalse);
    expect(secondAcceptRan, isFalse);
    expect(secondRejectRan, isTrue);
  });
}
