#ifndef SCENE_SERIALIZATION_H
#define SCENE_SERIALIZATION_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QColor>
#include <QString>
#include <QUuid>
#include <memory>
#include "../model/scene.h"
#include "../model/sceneColorificator.h"

/* -------------------------------------------------------------------------
 *  Helper utilities
 * --------------------------------------------------------------------- */
namespace detail {
// ---- helpers for <vector> ↔ QJsonArray --------------------------------
inline QJsonArray vecToJsonArray(const std::vector<double>& v)
{
    QJsonArray arr;
    for (double d : v) arr.append(d);
    return arr;
}
inline std::vector<double> jsonArrayToVec(const QJsonArray& arr)
{
    std::vector<double> v;  v.reserve(arr.size());
    for (const auto& e : arr) v.push_back(e.toDouble());
    return v;
}

// ---- QColor ↔ QString (#RRGGBBAA) -------------------------------------
inline QString colorToString(const QColor& c)
{
    return c.name(QColor::HexArgb);
}
inline QColor stringToColor(const QString& s)
{
    QColor c(s);  if (!c.isValid()) c = SceneColorificator::defaultColor;  return c;
}

// ---- QUuid ↔ QString ---------------------------------------------------
inline QString uidToString(const QUuid& id) { return id.toString(QUuid::WithoutBraces); }
inline QUuid   stringToUid(const QString& s){ return QUuid(s); }

} // namespace detail

/* -------------------------------------------------------------------------
 *  SceneSerializer (public façade)
 * --------------------------------------------------------------------- */
class SceneSerializer
{
public:
    /**
     * @brief Serialises a Scene together with its SceneColorificator to JSON.
     * @param scene            Scene holding the geometry/topology.
     * @param colorificator    Object holding the colours for each SceneObject.
     * @return QJsonDocument   A ready‑to‑save JSON document.
     */
    static QJsonDocument toJson(const Scene& scene,
                                const SceneColorificator& colorificator)
    {
        QJsonObject root;
        root.insert("sceneDimension", static_cast<int>(scene.getSceneDimension()));

        /* ---------- objects ------------------------------------------ */
        QJsonArray jObjects;
        for (const auto& weakObj : scene.getAllObjects()) {
            if (auto obj = weakObj.lock()) jObjects.append(sceneObjectToJson(*obj));
        }
        root.insert("objects", jObjects);

        /* ---------- colours ------------------------------------------ */
        QJsonObject jColors;
        for (const auto& weakObj : scene.getAllObjects()) {
            if (auto obj = weakObj.lock()) {
                QColor c = colorificator.getColorForObject(obj->uid);
                jColors.insert(detail::uidToString(obj->uid), detail::colorToString(c));
            }
        }
        root.insert("colors", jColors);
        return QJsonDocument(root);
    }

    /**
     * @brief Deserialises the JSON back into a Scene and its SceneColorificator.
     *        All existing content in @p scene and @p colorificator is cleared.
     */
    static void fromJson(const QJsonDocument& doc,
                         Scene&               scene,
                         SceneColorificator&  colorificator)
    {
        scene = Scene{};                         // reset
        colorificator = SceneColorificator{};    // reset

        QJsonObject root = doc.object();
        scene.setSceneDimension(root.value("sceneDimension").toInt(3));

        /* ---------- objects --------------------------------------------- */
        QJsonArray jObjects = root.value("objects").toArray();
        for (const auto& jVal : jObjects) {
            auto          obj  = jsonToSceneObject(jVal.toObject());
            const QUuid&  uid  = obj->uid;
            scene.addObject(uid, obj->id, obj->name, obj->shape,
                            obj->projection, obj->rotators,
                            obj->scale, obj->offset);
        }

        /* ---------- colour map ----------------------------------------- */
        QJsonObject jColors = root.value("colors").toObject();
        for (auto it = jColors.begin(), end = jColors.end(); it != end; ++it) {
            QUuid id = detail::stringToUid(it.key());
            colorificator.setColorForObject(id, detail::stringToColor(it.value().toString()));
        }
    }

    /* ===================== single object ============================= */
    /** Serialise just one SceneObject + its colour */
    static QJsonObject objectToJson(const SceneObject& obj, const QColor& color = SceneColorificator::defaultColor)
    {
        QJsonObject j = sceneObjectToJson(obj);
        if (color != SceneColorificator::defaultColor)
            j.insert("color", detail::colorToString(color));
        return j;
    }

    /**
     * @brief Creates a SceneObject (deep copy) from the JSON fragment.
     * @param jObj        Input JSON.
     * @param colorOut    Optional: receives the colour stored (or default).
     * @return            A shared_ptr<SceneObject> ready to insert into a Scene.
     */
    static std::shared_ptr<SceneObject> objectFromJson(const QJsonObject& jObj, QColor* colorOut = nullptr)
    {
        auto obj = jsonToSceneObject(jObj);
        if (colorOut) {
            if (jObj.contains("color")) *colorOut = detail::stringToColor(jObj.value("color").toString());
            else *colorOut = SceneColorificator::defaultColor;
        }
        return obj;
    }

private: /* ----------- per‑type helpers ---------------------------------- */
    static QJsonObject sceneObjectToJson(const SceneObject& obj)
    {
        QJsonObject jObj;
        jObj.insert("uid",       detail::uidToString(obj.uid));
        jObj.insert("id",        obj.id);
        jObj.insert("name",      obj.name);
        jObj.insert("shape",     shapeToJson(*obj.shape));

        // projection (type + params) ----------------------------------
        if (obj.projection) jObj.insert("projection", projectionToJson(obj.projection.get()));

        // rotators -----------------------------------------------------
        QJsonArray jRotators;
        for (const auto& r : obj.rotators) jRotators.append(rotatorToJson(r));
        jObj.insert("rotators", jRotators);

        // scale / offset ----------------------------------------------
        jObj.insert("scale",  detail::vecToJsonArray(obj.scale));
        jObj.insert("offset", detail::vecToJsonArray(obj.offset));
        return jObj;
    }

    static std::shared_ptr<SceneObject> jsonToSceneObject(const QJsonObject& jObj)
    {
        auto obj          = std::make_shared<SceneObject>();
        obj->uid          = detail::stringToUid(jObj.value("uid").toString());
        obj->id           = jObj.value("id").toInt();
        obj->name         = jObj.value("name").toString();
        obj->shape        = std::make_shared<NDShape>( jsonToShape(jObj.value("shape").toObject()) );

        // projection --------------------------------------------------
        if (jObj.contains("projection"))
            obj->projection = jsonToProjection(jObj.value("projection").toObject());

        // rotators ----------------------------------------------------
        QJsonArray jRotators = jObj.value("rotators").toArray();
        for (const auto& jr : jRotators) obj->rotators.push_back(jsonToRotator(jr.toObject()));

        // scale & offset ---------------------------------------------
        obj->scale  = detail::jsonArrayToVec(jObj.value("scale").toArray());
        obj->offset = detail::jsonArrayToVec(jObj.value("offset").toArray());
        return obj;
    }

    /* ---- NDShape -------------------------------------------------------- */
    static QJsonObject shapeToJson(const NDShape& shape)
    {
        QJsonObject jShape;
        jShape.insert("dim", static_cast<int>(shape.getDimension()));

        // vertices
        QJsonArray jVerts;
        for (auto [vid, coords] : shape.getAllVertices()) {
            QJsonObject jv;
            jv.insert("id", static_cast<int>(vid));
            jv.insert("coords", detail::vecToJsonArray(coords));
            jVerts.append(jv);
        }
        jShape.insert("vertices", jVerts);

        // edges
        QJsonArray jEdges;
        for (auto [a,b] : shape.getEdges()) {
            QJsonArray e;  e.append(static_cast<int>(a)); e.append(static_cast<int>(b));
            jEdges.append(e);
        }
        jShape.insert("edges", jEdges);
        return jShape;
    }

    static NDShape jsonToShape(const QJsonObject& jShape)
    {
        NDShape shape(jShape.value("dim").toInt());

        // vertices (must be read before edges)
        QJsonArray jVerts = jShape.value("vertices").toArray();
        for (const auto& jv : jVerts) {
            QJsonObject vv = jv.toObject();
            auto id        = static_cast<std::size_t>(vv.value("id").toInt());
            auto coords    = detail::jsonArrayToVec(vv.value("coords").toArray());
            // use private API via cloning trick: add temp id order preserved
            while (shape.getAllVertices().size() <= id) shape.addVertex(coords); // ensures sequential
            shape.setVertexCoords(id, coords);
        }

        // edges ------------------------------------------------------
        QJsonArray jEdges = jShape.value("edges").toArray();
        for (const auto& je : jEdges) {
            QJsonArray pair = je.toArray();
            shape.addEdge(pair.at(0).toInt(), pair.at(1).toInt());
        }
        return shape;
    }

    /* ---- Projection ----------------------------------------------------- */
    static QJsonObject projectionToJson(const Projection* proj)
    {
        QJsonObject j;
        if (auto pp = dynamic_cast<const PerspectiveProjection*>(proj)) {
            double dist = const_cast<PerspectiveProjection*>(pp)->getDistance(); // getDistance not const
            j.insert("type", "Perspective");
            j.insert("distance", dist);
        } else if (dynamic_cast<const OrthographicProjection*>(proj)) {
            j.insert("type", "Orthographic");
        } else if (dynamic_cast<const StereographicProjection*>(proj)) {
            j.insert("type", "Stereographic");
        }
        return j;
    }

    static std::shared_ptr<Projection> jsonToProjection(const QJsonObject& j)
    {
        const QString type = j.value("type").toString();
        if (type == "Perspective") {
            return std::make_shared<PerspectiveProjection>( j.value("distance").toDouble() );
        } else if (type == "Orthographic") {
            return std::make_shared<OrthographicProjection>();
        } else if (type == "Stereographic") {
            return std::make_shared<StereographicProjection>();
        }
        return nullptr; // unknown
    }

    /* ---- Rotator -------------------------------------------------------- */
    static QJsonObject rotatorToJson(const Rotator& r)
    {
        QJsonObject j;
        j.insert("axis1", static_cast<int>(r.axis1()));
        j.insert("axis2", static_cast<int>(r.axis2()));
        j.insert("angle", r.angle());
        return j;
    }
    static Rotator jsonToRotator(const QJsonObject& j)
    {
        return Rotator(static_cast<std::size_t>(j.value("axis1").toInt()),
                       static_cast<std::size_t>(j.value("axis2").toInt()),
                       j.value("angle").toDouble());
    }
};

#endif // SCENE_SERIALIZATION_H
