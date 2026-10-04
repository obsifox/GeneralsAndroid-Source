package com.obsifox.generals.net

import android.content.Context
import android.net.wifi.WifiManager

/**
 * Keeps hotspot/LAN traffic alive:
 * - MulticastLock: some Android Wi-Fi drivers drop broadcast/multicast packets
 *   unless a multicast lock is held (essential for UDP discovery).
 * - WifiLock: prevents Wi-Fi doze from killing the lobby socket.
 * Locks are acquired with reference counting semantics via application context.
 */
object HotspotNet {

    private var multicastLock: WifiManager.MulticastLock? = null
    private var wifiLock: WifiManager.WifiLock? = null

    @Synchronized
    fun acquire(context: Context) {
        val app = context.applicationContext
        if (multicastLock == null) {
            val wm = app.getSystemService(Context.WIFI_SERVICE) as? WifiManager
            if (wm != null) {
                multicastLock = wm.createMulticastLock("generals-lobby-mdns").apply {
                    setReferenceCounted(false)
                    acquire()
                }
                wifiLock = wm.createWifiLock(WifiManager.WIFI_MODE_FULL_HIGH_PERF, "generals-lobby").apply {
                    setReferenceCounted(false)
                    acquire()
                }
            }
        }
    }

    @Synchronized
    fun release() {
        try { multicastLock?.release() } catch (t: Throwable) {}
        try { wifiLock?.release() } catch (t: Throwable) {}
        multicastLock = null
        wifiLock = null
    }
}
