// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter/material.dart';
import 'package:flutter/painting.dart';
import 'package:flutter/rendering.dart';

class DragData {
  DragData(this.text);

  final String text;
}

class ExampleDragTarget extends StatefulComponent {
  ExampleDragTargetState createState() => new ExampleDragTargetState();
}

class ExampleDragTargetState extends State<ExampleDragTarget> {
  String _text = 'Drag Target';

  void _handleAccept(DragData data) {
    setState(() {
      _text = 'dropped: ${data.text}';
    });
  }

  Widget build(BuildContext context) {
    return new DragTarget<DragData>(
      onAccept: _handleAccept,
      builder: (BuildContext context, List<DragData> data, _) {
        return new Container(
          height: 100.0,
          margin: new EdgeDims.all(10.0),
          decoration: new BoxDecoration(
            border: new Border.all(
              width: 3.0,
              color: data.isEmpty ? Colors.white : Colors.blue[500]
            ),
            backgroundColor: data.isEmpty ? Colors.grey[500] : Colors.green[500]
          ),
          child: new Center(
            child: new Text(_text)
          )
        );
      }
    );
  }
}

class Dot extends StatelessComponent {
  Dot({ Key key, this.color, this.size }) : super(key: key);
  final Color color;
  final double size;
  Widget build(BuildContext context) {
    return new Container(
      width: size,
      height: size,
      decoration: new BoxDecoration(
        borderRadius: 10.0,
        backgroundColor: color
      )
    );
  }
}

class ExampleDragSource extends StatelessComponent {
  ExampleDragSource({ Key key, this.name, this.color }) : super(key: key);
  final String name;
  final Color color;

  static const kDotSize = 50.0;
  static const kFingerSize = 50.0;

  Widget build(BuildContext context) {
    return new Draggable(
      data: new DragData(name),
      child: new Dot(color: color, size: kDotSize),
      feedback: new Transform(
        transform: new Matrix4.identity()..translate(-kDotSize / 2.0, -(kDotSize / 2.0 + kFingerSize)),
        child: new Opacity(
          opacity: 0.75,
          child: new Dot(color: color, size: kDotSize)
        )
      ),
      feedbackOffset: const Offset(0.0, -kFingerSize),
      dragAnchor: DragAnchor.pointer
    );
  }
}

class DragAndDropApp extends StatelessComponent {
  Widget build(BuildContext context) {
    return new Scaffold(
      toolBar: new ToolBar(
        center: new Text('Drag and Drop Flutter Demo')
      ),
      body: new DefaultTextStyle(
        style: Theme.of(context).text.body1.copyWith(textAlign: TextAlign.center),
        child: new Column(<Widget>[
          new Flexible(child: new Row(<Widget>[
              new ExampleDragSource(name: 'Orange', color: const Color(0xFFFF9000)),
              new ExampleDragSource(name: 'Teal', color: const Color(0xFF00FFFF)),
              new ExampleDragSource(name: 'Yellow', color: const Color(0xFFFFF000)),
            ],
            alignItems: FlexAlignItems.center,
            justifyContent: FlexJustifyContent.spaceAround
          )),
          new Flexible(child: new Row(<Widget>[
            new Flexible(child: new ExampleDragTarget()),
            new Flexible(child: new ExampleDragTarget()),
            new Flexible(child: new ExampleDragTarget()),
            new Flexible(child: new ExampleDragTarget()),
          ])),
        ])
      )
    );
  }
}

void main() {
  runApp(new MaterialApp(
    title: 'Drag and Drop Flutter Demo',
    routes: <String, RouteBuilder>{
     '/': (RouteArguments args) => new DragAndDropApp()
    }
  ));
}
