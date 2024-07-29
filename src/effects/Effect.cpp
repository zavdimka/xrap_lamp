#include "Effect.h"

int random(int start, int end){
    return (qrand() % (end - start)) + start;
}

Effect::Effect(Image im, const QString &id):
    im(im)
{
    settings.id = id;
    settings.name = id;
}

Effect::~Effect()
{

}

void Effect::Process() {
    tick();
}

void Effect::initialize(const QJsonObject &json)
{
    update(json);
}

void Effect::update(const QJsonObject &json)
{
    if (json.contains("i")) {
        settings.id = json["i"].toString() ;
    }
    if (json.contains("n")) {
        settings.name = json["n"].toString();
    }
    if (json.contains("name")) {
        settings.name = json["name"].toString();
    }
    if (json.contains("s")) {
        settings.speed = json["s"].toInt();
    }
    if (json.contains("speed")) {
        settings.speed = json["speed"].toInt();
    }
    if (json.contains("b")) {
        settings.brightness = json["b"].toInt();
    }
    if (json.contains("brightness")) {
        settings.brightness = json["brightness"].toInt();
    }
    if (json.contains("l")) {
        settings.scale = json["l"].toInt();
    }
    if (json.contains("scale")) {
        settings.scale = json["scale"].toInt();
    }
}