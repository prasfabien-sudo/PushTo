import Capacitor
import CoreMotion
import Foundation
import Network
import NetworkExtension
import Photos
import Security
import SQLite3

@objc(DobsonNativePlugin)
public class DobsonNativePlugin: CAPPlugin, CAPBridgedPlugin {
    public let identifier = "DobsonNativePlugin"
    public let jsName = "DobsonNative"
    public let pluginMethods: [CAPPluginMethod] = [
        CAPPluginMethod(name: "connect", returnType: CAPPluginReturnPromise),
        CAPPluginMethod(name: "disconnect", returnType: CAPPluginReturnPromise),
        CAPPluginMethod(name: "getWifiState", returnType: CAPPluginReturnPromise),
        CAPPluginMethod(name: "getEncoderStatus", returnType: CAPPluginReturnPromise),
        CAPPluginMethod(name: "startSensors", returnType: CAPPluginReturnPromise),
        CAPPluginMethod(name: "stopSensors", returnType: CAPPluginReturnPromise),
        CAPPluginMethod(name: "initializeCatalog", returnType: CAPPluginReturnPromise),
        CAPPluginMethod(name: "getCatalog", returnType: CAPPluginReturnPromise),
        CAPPluginMethod(name: "searchCatalog", returnType: CAPPluginReturnPromise),
        CAPPluginMethod(name: "saveAstrometryApiKey", returnType: CAPPluginReturnPromise),
        CAPPluginMethod(name: "getAstrometryApiKey", returnType: CAPPluginReturnPromise),
        CAPPluginMethod(name: "clearAstrometryApiKey", returnType: CAPPluginReturnPromise),
        CAPPluginMethod(name: "saveAstroPhoto", returnType: CAPPluginReturnPromise)
    ]

    private let motionManager = CMMotionManager()
    private var connectedWifiSsid: String?
    private let wifiStateQueue = DispatchQueue(label: "DobsonWifiState")
    private var database: DobsonCatalogDatabase?
    private let motionQueue: OperationQueue = {
        let queue = OperationQueue()
        queue.name = "DobsonSensors"
        queue.qualityOfService = .userInteractive
        queue.maxConcurrentOperationCount = 1
        return queue
    }()

    @objc func saveAstroPhoto(_ call: CAPPluginCall) {
        guard let base64 = call.getString("base64"),
              let data = Data(base64Encoded: base64), !data.isEmpty,
              let filename = call.getString("filename"),
              filename.range(of: #"^ciel-[0-9]+\.jpg$"#, options: .regularExpression) != nil else {
            call.reject("Photo ou nom de fichier invalide.")
            return
        }

        PHPhotoLibrary.requestAuthorization(for: .readWrite) { authorization in
            guard authorization == .authorized else {
                call.reject("Autorisez l'accès complet aux photos pour enregistrer dans l'album AstroPhoto.")
                return
            }
            let options = PHFetchOptions()
            options.predicate = NSPredicate(format: "title = %@", "AstroPhoto")
            let albums = PHAssetCollection.fetchAssetCollections(with: .album, subtype: .albumRegular, options: options)
            let album = albums.firstObject
            var addedToAlbum = false
            PHPhotoLibrary.shared().performChanges({
                let albumChange: PHAssetCollectionChangeRequest
                if let album = album {
                    guard let change = PHAssetCollectionChangeRequest(for: album) else { return }
                    albumChange = change
                } else {
                    albumChange = PHAssetCollectionChangeRequest.creationRequestForAssetCollection(withTitle: "AstroPhoto")
                }
                let creation = PHAssetCreationRequest.forAsset()
                let resourceOptions = PHAssetResourceCreationOptions()
                resourceOptions.originalFilename = filename
                creation.addResource(with: .photo, data: data, options: resourceOptions)
                guard let asset = creation.placeholderForCreatedAsset else {
                    return
                }
                albumChange.addAssets([asset] as NSArray)
                addedToAlbum = true
            }) { success, error in
                if success && addedToAlbum {
                    call.resolve()
                } else {
                    call.reject("Impossible d'enregistrer la photo dans AstroPhoto.", nil, error)
                }
            }
        }
    }

    @objc func connect(_ call: CAPPluginCall) {
        guard let ssid = call.getString("ssid")?.trimmingCharacters(in: .whitespacesAndNewlines),
              !ssid.isEmpty else {
            call.reject("Le nom du réseau Wi-Fi est obligatoire.")
            return
        }

        let password = call.getString("password") ?? ""
        let configuration: NEHotspotConfiguration
        if password.isEmpty {
            configuration = NEHotspotConfiguration(ssid: ssid)
        } else {
            guard password.count >= 8 else {
                call.reject("Le mot de passe WPA2 doit contenir au moins 8 caractères.")
                return
            }
            configuration = NEHotspotConfiguration(ssid: ssid, passphrase: password, isWEP: false)
        }
        configuration.joinOnce = false

        NEHotspotConfigurationManager.shared.apply(configuration) { error in
            if let error = error as? NEHotspotConfigurationError,
               error.code == .alreadyAssociated {
                self.wifiStateQueue.sync { self.connectedWifiSsid = ssid }
                call.resolve(["ssid": ssid])
                self.notifyListeners("wifiState", data: ["connected": true, "ssid": ssid])
                return
            }
            if let error = error {
                call.reject("iOS n'a pas pu rejoindre le réseau Wi-Fi. Vérifiez le réseau et acceptez la demande système.", nil, error)
                return
            }
            self.wifiStateQueue.sync { self.connectedWifiSsid = ssid }
            call.resolve(["ssid": ssid])
            self.notifyListeners("wifiState", data: ["connected": true, "ssid": ssid])
        }
    }

    @objc func disconnect(_ call: CAPPluginCall) {
        guard let ssid = call.getString("ssid"), !ssid.isEmpty else {
            call.reject("Le nom du réseau Wi-Fi est obligatoire.")
            return
        }
        NEHotspotConfigurationManager.shared.removeConfiguration(forSSID: ssid)
        wifiStateQueue.sync { connectedWifiSsid = nil }
        call.resolve()
        notifyListeners("wifiState", data: ["connected": false, "ssid": ssid])
    }

    @objc func getWifiState(_ call: CAPPluginCall) {
        let ssid = wifiStateQueue.sync { connectedWifiSsid }
        call.resolve(["connected": ssid != nil, "ssid": ssid ?? ""])
    }

    @objc func getEncoderStatus(_ call: CAPPluginCall) {
        let queue = DispatchQueue(label: "DobsonEncoderRead")
        let connection = NWConnection(
            host: NWEndpoint.Host("192.168.4.1"),
            port: NWEndpoint.Port(rawValue: 4030)!,
            using: .tcp
        )
        var completed = false
        var stage = "Connexion TCP"

        func finishWithError(_ message: String, _ error: Error? = nil) {
            guard !completed else { return }
            completed = true
            connection.cancel()
            call.reject(message, nil, error)
        }

        func receiveLine(_ accumulated: Data = Data(), completion: @escaping (Result<String, Error>) -> Void) {
            connection.receive(minimumIncompleteLength: 1, maximumLength: 128) { content, _, isComplete, error in
                if let error = error {
                    completion(.failure(error))
                    return
                }
                var received = accumulated
                if let content = content {
                    received.append(content)
                }
                if let separator = received.firstIndex(of: 13) {
                    guard let line = String(data: Data(received[..<separator]), encoding: .utf8) else {
                        completion(.failure(NSError(domain: "DobsonEncoder", code: 1, userInfo: [
                            NSLocalizedDescriptionKey: "Réponse encodeurs illisible."
                        ])))
                        return
                    }
                    completion(.success(line))
                } else if isComplete || received.count >= 128 {
                    completion(.failure(NSError(domain: "DobsonEncoder", code: 2, userInfo: [
                        NSLocalizedDescriptionKey: "Réponse encodeurs incomplète."
                    ])))
                } else {
                    receiveLine(received, completion: completion)
                }
            }
        }

        func sendRequest(_ command: String, completion: @escaping (Result<String, Error>) -> Void) {
            connection.send(content: Data(command.utf8), completion: .contentProcessed { error in
                if let error = error {
                    completion(.failure(error))
                } else {
                    receiveLine(completion: completion)
                }
            })
        }

        let timeout = DispatchWorkItem {
            finishWithError("\(stage) : délai dépassé.")
        }
        queue.asyncAfter(deadline: .now() + 2, execute: timeout)
        connection.stateUpdateHandler = { state in
            guard !completed else { return }
            switch state {
            case .ready:
                connection.stateUpdateHandler = nil
                stage = "Lecture de la position (Q)"
                sendRequest("Q") { positionResult in
                    switch positionResult {
                    case .failure(let error):
                        finishWithError("Impossible de lire la position des encodeurs.", error)
                    case .success(let positionLine):
                        stage = "Lecture de la résolution (a)"
                        sendRequest("a") { resolutionResult in
                            switch resolutionResult {
                            case .failure(let error):
                                finishWithError("Impossible de lire la résolution des encodeurs.", error)
                            case .success(let resolutionLine):
                                let position = positionLine.split(whereSeparator: \.isWhitespace).compactMap { Int($0) }
                                let resolution = resolutionLine.split(whereSeparator: \.isWhitespace).compactMap { Int($0) }
                                guard position.count == 2, resolution.count == 2,
                                      resolution[0] > 0, resolution[1] > 0 else {
                                    finishWithError("Réponse encodeurs invalide.")
                                    return
                                }
                                guard !completed else { return }
                                completed = true
                                timeout.cancel()
                                connection.cancel()
                                call.resolve([
                                    "rawAZ": position[0],
                                    "rawALT": position[1],
                                    "ticksAZ": resolution[0],
                                    "ticksALT": resolution[1]
                                ])
                            }
                        }
                    }
                }
            case .failed(let error):
                finishWithError("Impossible de se connecter aux encodeurs TCP.", error)
            default:
                break
            }
        }
        connection.start(queue: queue)
    }

    @objc func startSensors(_ call: CAPPluginCall) {
        guard motionManager.isDeviceMotionAvailable else {
            call.reject("Ce téléphone ne fournit pas de données de mouvement.")
            return
        }

        let interval = call.getDouble("intervalMs") ?? 10
        motionManager.deviceMotionUpdateInterval = min(max(interval, 10), 100) / 1000
        let frames = CMMotionManager.availableAttitudeReferenceFrames()
        let frame: CMAttitudeReferenceFrame = frames.contains(.xMagneticNorthZVertical)
            ? .xMagneticNorthZVertical
            : .xArbitraryCorrectedZVertical

        motionManager.startDeviceMotionUpdates(using: frame, to: motionQueue) { [weak self] motion, error in
            guard let self = self else { return }
            if let error = error {
                self.notifyListeners("sensorError", data: ["message": error.localizedDescription])
                return
            }
            guard let motion = motion else { return }
            let degrees = 180 / Double.pi
            let rotation = motion.attitude.rotationMatrix
            let north = rotation.m12
            let east = -rotation.m22
            let up = rotation.m32
            let azimuth = (atan2(east, north) * degrees + 360).truncatingRemainder(dividingBy: 360)
            let altitude = asin(max(-1, min(1, up))) * degrees
            let cameraNorth = -rotation.m13
            let cameraEast = rotation.m23
            let cameraUp = -rotation.m33
            let cameraAzimuth = (atan2(cameraEast, cameraNorth) * degrees + 360).truncatingRemainder(dividingBy: 360)
            let cameraAltitude = asin(max(-1, min(1, cameraUp))) * degrees
            let timestamp = motion.timestamp * 1000
            self.notifyListeners("orientation", data: [
                "azimuth": azimuth,
                "altitude": altitude,
                "cameraAzimuth": cameraAzimuth,
                "cameraAltitude": cameraAltitude,
                "pitch": motion.attitude.pitch * degrees,
                "roll": motion.attitude.roll * degrees,
                "timestamp": timestamp,
                "headingAvailable": frame == .xMagneticNorthZVertical
            ])
        }
        call.resolve()
    }

    @objc func stopSensors(_ call: CAPPluginCall) {
        motionManager.stopDeviceMotionUpdates()
        call.resolve()
    }

    @objc func saveAstrometryApiKey(_ call: CAPPluginCall) {
        guard let apiKey = call.getString("apiKey")?.trimmingCharacters(in: .whitespacesAndNewlines),
              !apiKey.isEmpty,
              let data = apiKey.data(using: .utf8) else {
            call.reject("La clé API Astrometry.net est vide.")
            return
        }
        var query = astrometryKeychainQuery()
        query[kSecValueData as String] = data
        SecItemDelete(astrometryKeychainQuery() as CFDictionary)
        let status = SecItemAdd(query as CFDictionary, nil)
        guard status == errSecSuccess else {
            call.reject("Impossible d'enregistrer la clé API Astrometry.net dans le trousseau (code \(status)).")
            return
        }
        call.resolve()
    }

    @objc func getAstrometryApiKey(_ call: CAPPluginCall) {
        var query = astrometryKeychainQuery()
        query[kSecReturnData as String] = true
        query[kSecMatchLimit as String] = kSecMatchLimitOne
        var result: CFTypeRef?
        let status = SecItemCopyMatching(query as CFDictionary, &result)
        if status == errSecItemNotFound {
            call.resolve(["apiKey": ""])
            return
        }
        guard status == errSecSuccess,
              let data = result as? Data,
              let apiKey = String(data: data, encoding: .utf8) else {
            call.reject("Impossible de lire la clé API Astrometry.net dans le trousseau (code \(status)).")
            return
        }
        call.resolve(["apiKey": apiKey])
    }

    @objc func clearAstrometryApiKey(_ call: CAPPluginCall) {
        let status = SecItemDelete(astrometryKeychainQuery() as CFDictionary)
        guard status == errSecSuccess || status == errSecItemNotFound else {
            call.reject("Impossible d'effacer la clé API Astrometry.net du trousseau (code \(status)).")
            return
        }
        call.resolve()
    }

    @objc func initializeCatalog(_ call: CAPPluginCall) {
        guard let revision = call.getString("revision"),
              let rawCatalog = call.getString("catalogJson"),
              let data = rawCatalog.data(using: .utf8) else {
            call.reject("Le catalogue ou sa révision est absent.")
            return
        }
        do {
            try catalogDatabase().initialize(revision: revision, data: data)
            call.resolve()
        } catch {
            call.reject("Impossible d'enregistrer le catalogue dans SQLite.", nil, error)
        }
    }

    @objc func getCatalog(_ call: CAPPluginCall) {
        do {
            call.resolve(["objects": try catalogDatabase().query(search: nil, limit: 0)])
        } catch {
            call.reject("Impossible de lire le catalogue SQLite.", nil, error)
        }
    }

    @objc func searchCatalog(_ call: CAPPluginCall) {
        let query = call.getString("query")?.trimmingCharacters(in: .whitespacesAndNewlines) ?? ""
        let limit = min(max(call.getInt("limit") ?? 100, 1), 1000)
        do {
            call.resolve(["objects": try catalogDatabase().query(search: query, limit: limit)])
        } catch {
            call.reject("La recherche dans le catalogue SQLite a échoué.", nil, error)
        }
    }

    private func catalogDatabase() throws -> DobsonCatalogDatabase {
        if let database = database { return database }
        let openedDatabase = try DobsonCatalogDatabase()
        database = openedDatabase
        return openedDatabase
    }

    private func astrometryKeychainQuery() -> [String: Any] {
        [
            kSecClass as String: kSecClassGenericPassword,
            kSecAttrService as String: "fr.dobsonpushto.mobile",
            kSecAttrAccount as String: "astrometry-api-key"
        ]
    }
}

private final class DobsonCatalogDatabase {
    private var handle: OpaquePointer?
    private let lock = NSLock()

    init() throws {
        let directory = FileManager.default.urls(for: .applicationSupportDirectory, in: .userDomainMask)[0]
        try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
        let url = directory.appendingPathComponent("dobson-catalog.sqlite")
        guard sqlite3_open(url.path, &handle) == SQLITE_OK else { throw databaseError() }
        do {
            try execute("CREATE TABLE IF NOT EXISTS objects (name TEXT NOT NULL, common_name TEXT NOT NULL, type TEXT NOT NULL, ra REAL NOT NULL, dec REAL NOT NULL, mag REAL NOT NULL)")
            try execute("CREATE INDEX IF NOT EXISTS objects_name_idx ON objects(name COLLATE NOCASE)")
            try execute("CREATE INDEX IF NOT EXISTS objects_type_idx ON objects(type COLLATE NOCASE)")
            try execute("CREATE TABLE IF NOT EXISTS catalog_meta (key TEXT PRIMARY KEY, value TEXT NOT NULL)")
        } catch {
            if let handle = handle { sqlite3_close(handle) }
            handle = nil
            throw error
        }
    }

    deinit {
        if let handle = handle { sqlite3_close(handle) }
    }

    func initialize(revision: String, data: Data) throws {
        lock.lock()
        defer { lock.unlock() }
        guard let handle = handle else { throw databaseError() }

        var statement: OpaquePointer?
        try prepare("SELECT value FROM catalog_meta WHERE key = 'revision'", into: &statement, db: handle)
        defer { sqlite3_finalize(statement) }
        let revisionResult = sqlite3_step(statement)
        if revisionResult == SQLITE_ROW,
           let text = sqlite3_column_text(statement, 0),
           String(cString: text) == revision {
            return
        }
        if revisionResult != SQLITE_ROW && revisionResult != SQLITE_DONE {
            throw databaseError()
        }

        guard let objects = try JSONSerialization.jsonObject(with: data) as? [[String: Any]] else {
            throw NSError(domain: "DobsonCatalog", code: 1, userInfo: [NSLocalizedDescriptionKey: "Le format du catalogue est invalide."])
        }
        try execute("BEGIN TRANSACTION")
        do {
            try execute("DELETE FROM objects")
            for object in objects {
                guard let name = object["name"] as? String,
                      let commonName = object["commonName"] as? String,
                      let type = object["type"] as? String,
                      let ra = object["ra"] as? NSNumber,
                      let dec = object["dec"] as? NSNumber,
                      let mag = object["mag"] as? NSNumber else {
                    throw NSError(domain: "DobsonCatalog", code: 2, userInfo: [NSLocalizedDescriptionKey: "Une entrée du catalogue est invalide."])
                }
                try insert(name: name, commonName: commonName, type: type, ra: ra.doubleValue, dec: dec.doubleValue, mag: mag.doubleValue)
            }
            try storeRevision(revision)
            try execute("COMMIT")
        } catch {
            try execute("ROLLBACK")
            throw error
        }
    }

    func query(search: String?, limit: Int) throws -> [[String: Any]] {
        lock.lock()
        defer { lock.unlock() }
        guard let handle = handle else { throw databaseError() }

        let filtered = !(search ?? "").isEmpty
        let sql = filtered
            ? "SELECT name, common_name, type, ra, dec, mag FROM objects WHERE name LIKE ? COLLATE NOCASE OR common_name LIKE ? COLLATE NOCASE OR type LIKE ? COLLATE NOCASE ORDER BY name LIMIT ?"
            : "SELECT name, common_name, type, ra, dec, mag FROM objects ORDER BY name"
        var statement: OpaquePointer?
        try prepare(sql, into: &statement, db: handle)
        defer { sqlite3_finalize(statement) }

        if filtered {
            let pattern = "%\(search ?? "")"
            let fullPattern = "\(pattern)%"
            sqlite3_bind_text(statement, 1, fullPattern, -1, SQLITE_TRANSIENT)
            sqlite3_bind_text(statement, 2, fullPattern, -1, SQLITE_TRANSIENT)
            sqlite3_bind_text(statement, 3, fullPattern, -1, SQLITE_TRANSIENT)
            sqlite3_bind_int(statement, 4, Int32(limit))
        }

        var objects: [[String: Any]] = []
        while true {
            let result = sqlite3_step(statement)
            if result == SQLITE_DONE { break }
            guard result == SQLITE_ROW else { throw databaseError() }
            objects.append([
                "name": String(cString: sqlite3_column_text(statement, 0)),
                "commonName": String(cString: sqlite3_column_text(statement, 1)),
                "type": String(cString: sqlite3_column_text(statement, 2)),
                "ra": sqlite3_column_double(statement, 3),
                "dec": sqlite3_column_double(statement, 4),
                "mag": sqlite3_column_double(statement, 5)
            ])
        }
        return objects
    }

    private func insert(name: String, commonName: String, type: String, ra: Double, dec: Double, mag: Double) throws {
        guard let handle = handle else { throw databaseError() }
        var statement: OpaquePointer?
        try prepare("INSERT INTO objects (name, common_name, type, ra, dec, mag) VALUES (?, ?, ?, ?, ?, ?)", into: &statement, db: handle)
        defer { sqlite3_finalize(statement) }
        sqlite3_bind_text(statement, 1, name, -1, SQLITE_TRANSIENT)
        sqlite3_bind_text(statement, 2, commonName, -1, SQLITE_TRANSIENT)
        sqlite3_bind_text(statement, 3, type, -1, SQLITE_TRANSIENT)
        sqlite3_bind_double(statement, 4, ra)
        sqlite3_bind_double(statement, 5, dec)
        sqlite3_bind_double(statement, 6, mag)
        guard sqlite3_step(statement) == SQLITE_DONE else { throw databaseError() }
    }

    private func storeRevision(_ revision: String) throws {
        guard let handle = handle else { throw databaseError() }
        var statement: OpaquePointer?
        try prepare("INSERT OR REPLACE INTO catalog_meta (key, value) VALUES ('revision', ?)", into: &statement, db: handle)
        defer { sqlite3_finalize(statement) }
        sqlite3_bind_text(statement, 1, revision, -1, SQLITE_TRANSIENT)
        guard sqlite3_step(statement) == SQLITE_DONE else { throw databaseError() }
    }

    private func execute(_ sql: String) throws {
        guard let handle = handle else { throw databaseError() }
        var errorMessage: UnsafeMutablePointer<CChar>?
        guard sqlite3_exec(handle, sql, nil, nil, &errorMessage) == SQLITE_OK else {
            let message = errorMessage.map { String(cString: $0) } ?? "SQLite error"
            sqlite3_free(errorMessage)
            throw NSError(domain: "DobsonCatalog", code: 3, userInfo: [NSLocalizedDescriptionKey: message])
        }
    }

    private func prepare(_ sql: String, into statement: inout OpaquePointer?, db: OpaquePointer) throws {
        guard sqlite3_prepare_v2(db, sql, -1, &statement, nil) == SQLITE_OK else {
            throw databaseError()
        }
    }

    private func databaseError() -> NSError {
        let message = handle.map { String(cString: sqlite3_errmsg($0)) } ?? "La base SQLite n'est pas disponible."
        return NSError(domain: "DobsonCatalog", code: 4, userInfo: [NSLocalizedDescriptionKey: message])
    }
}

private let SQLITE_TRANSIENT = unsafeBitCast(-1, to: sqlite3_destructor_type.self)
