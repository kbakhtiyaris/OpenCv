#define _CRT_SECURE_NO_WARNINGS
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <iostream>
#include <vector>
#include <windows.h>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <thread>
#include <chrono>
#include <direct.h>

// BALANCED Configuration - Fast + Good Detection
struct Config {
    std::string comPort = "COM6";
    std::string streamUrl = "http://192.168.1.103:8080/video";
    std::string saveFolder = "C:/Users/kbakhtiyar/Documents/document_tester/";

    // ADJUSTED detection parameters for lower resolution
    int minArea = 1500;              // REDUCED for 480x360 (was 3000)
    int maxArea = 300000;            // REDUCED for 480x360 (was 500000)
    double epsilonFactor = 0.02;
    int detectionTimeSeconds = 5;
    int cannyLow = 20;               // LOWERED for better edge detection
    int cannyHigh = 100;             // LOWERED for better edge detection

    bool autoEnhance = true;
    bool showPreview = false;        // Keep disabled for speed
    int qualityThreshold = 60;       // LOWERED threshold (was 70)

    // BALANCED camera settings
    int frameWidth = 480;
    int frameHeight = 360;
    int fps = 30;                    // REDUCED from 60 for stability
    int bufferSize = 1;              // Keep minimal for speed

    // BALANCED processing
    bool skipFrames = false;         // DISABLED - detect every frame
    int processEveryNthFrame = 1;    // Process EVERY frame for detection
    bool fastProcessing = false;     // DISABLED - use full processing
    bool useColorDetection = true;   // ENABLED - better document detection
};

// Same fast serial port
class SerialPort {
private:
    HANDLE hSerial;
    std::string portName;
    bool connected;

public:
    SerialPort() : hSerial(INVALID_HANDLE_VALUE), connected(false) {}

    bool init(const std::string& comPort, DWORD baud = CBR_115200) {
        portName = comPort;
        return reconnect(baud);
    }

    bool reconnect(DWORD baud = CBR_115200) {
        if (hSerial != INVALID_HANDLE_VALUE) {
            CloseHandle(hSerial);
        }

        std::string full = "\\.\\" + portName;
        hSerial = CreateFileA(full.c_str(), GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hSerial == INVALID_HANDLE_VALUE) {
            connected = false;
            return false;
        }

        DCB dcb;
        SecureZeroMemory(&dcb, sizeof(dcb));
        dcb.DCBlength = sizeof(dcb);

        if (!GetCommState(hSerial, &dcb)) {
            CloseHandle(hSerial);
            hSerial = INVALID_HANDLE_VALUE;
            connected = false;
            return false;
        }

        dcb.BaudRate = baud;
        dcb.ByteSize = 8;
        dcb.Parity = NOPARITY;
        dcb.StopBits = ONESTOPBIT;
        dcb.fDtrControl = DTR_CONTROL_ENABLE;
        dcb.fRtsControl = RTS_CONTROL_ENABLE;

        if (!SetCommState(hSerial, &dcb)) {
            CloseHandle(hSerial);
            hSerial = INVALID_HANDLE_VALUE;
            connected = false;
            return false;
        }

        COMMTIMEOUTS timeouts = { 0 };
        timeouts.ReadIntervalTimeout = 10;
        timeouts.ReadTotalTimeoutConstant = 10;
        timeouts.WriteTotalTimeoutConstant = 10;
        SetCommTimeouts(hSerial, &timeouts);

        connected = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        return true;
    }

    bool send(const std::string& msg) {
        if (!connected && !reconnect()) return false;

        DWORD bytesWritten = 0;
        BOOL ok = WriteFile(hSerial, msg.c_str(), (DWORD)msg.size(), &bytesWritten, NULL);
        FlushFileBuffers(hSerial);

        if (!ok || bytesWritten != msg.size()) {
            connected = false;
            return false;
        }
        return true;
    }

    bool isConnected() const { return connected; }

    ~SerialPort() {
        if (hSerial != INVALID_HANDLE_VALUE) {
            CloseHandle(hSerial);
        }
    }
};

struct QualityMetrics {
    double sharpness;
    double brightness;
    double contrast;
    int overallScore;
    bool isGoodQuality;
};

void ensureDirectoryExists(const std::string& path) {
    _mkdir(path.c_str());
}

// BALANCED document detection - fast but accurate
class BalancedDocumentDetector {
private:
    Config config;
    cv::Mat kernel;

public:
    BalancedDocumentDetector(const Config& cfg) : config(cfg) {
        kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    }

    // BALANCED preprocessing - fast but thorough
    cv::Mat balancedPreprocess(const cv::Mat& img) {
        cv::Mat gray, enhanced, blurred, edges, morph;

        // Convert to grayscale
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

        // RESTORED: CLAHE for better contrast (essential for detection)
        cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
        clahe->apply(gray, enhanced);

        // RESTORED: Bilateral filter for noise reduction
        cv::bilateralFilter(enhanced, blurred, 5, 50, 50); // Faster than original

        // RESTORED: Multi-scale edge detection (critical for detection)
        cv::Mat edges1, edges2;
        cv::Canny(blurred, edges1, config.cannyLow, config.cannyHigh);
        cv::Canny(blurred, edges2, config.cannyLow / 2, config.cannyHigh / 2);
        cv::bitwise_or(edges1, edges2, edges);

        // RESTORED: Enhanced morphological operations
        cv::dilate(edges, morph, kernel, cv::Point(-1, -1), 2);
        cv::erode(morph, morph, kernel, cv::Point(-1, -1), 1);
        cv::morphologyEx(morph, morph, cv::MORPH_CLOSE, kernel);

        return morph;
    }

    // RESTORED: Color-based paper detection (critical for documents)
    cv::Mat detectPaper(const cv::Mat& img) {
        cv::Mat hsv, mask, mask1, mask2;
        cv::cvtColor(img, hsv, cv::COLOR_BGR2HSV);

        // White paper range in HSV
        cv::inRange(hsv, cv::Scalar(0, 0, 180), cv::Scalar(180, 30, 255), mask1);

        // Light colored surfaces
        cv::inRange(hsv, cv::Scalar(0, 0, 150), cv::Scalar(180, 50, 255), mask2);

        cv::bitwise_or(mask1, mask2, mask);

        // Morphological operations to clean up
        cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel);
        cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(15, 15)));

        return mask;
    }

    std::vector<cv::Point> reorderPoints(const std::vector<cv::Point>& pts) {
        if (pts.size() != 4) return pts;

        std::vector<cv::Point> ordered(4);
        std::vector<double> sums, diffs;

        for (size_t i = 0; i < pts.size(); i++) {
            sums.push_back(pts[i].x + pts[i].y);
            diffs.push_back(pts[i].x - pts[i].y);
        }

        ordered[0] = pts[std::min_element(sums.begin(), sums.end()) - sums.begin()];
        ordered[1] = pts[std::min_element(diffs.begin(), diffs.end()) - diffs.begin()];
        ordered[2] = pts[std::max_element(diffs.begin(), diffs.end()) - diffs.begin()];
        ordered[3] = pts[std::max_element(sums.begin(), sums.end()) - sums.begin()];

        return ordered;
    }

    // IMPROVED: Better validation for smaller resolution
    bool isValidDocument(const std::vector<cv::Point>& contour, const cv::Size& imgSize) {
        double area = cv::contourArea(contour);
        if (area < config.minArea || area > config.maxArea) return false;

        std::vector<cv::Point> approx;
        cv::approxPolyDP(contour, approx, config.epsilonFactor * cv::arcLength(contour, true), true);
        if (approx.size() != 4) return false;

        // Check aspect ratio (should be reasonable for documents)
        cv::Rect bbox = cv::boundingRect(approx);
        double aspectRatio = (double)bbox.width / bbox.height;
        if (aspectRatio < 0.2 || aspectRatio > 5.0) return false; // More lenient

        // ADJUSTED: Smaller margin for lower resolution
        int margin = 10; // Reduced from 20
        for (size_t i = 0; i < approx.size(); i++) {
            if (approx[i].x < margin || approx[i].y < margin ||
                approx[i].x > imgSize.width - margin || approx[i].y > imgSize.height - margin) {
                return false;
            }
        }

        // Check if contour is convex enough
        if (!cv::isContourConvex(approx)) {
            return false;
        }

        return true;
    }

    // RESTORED: Find best document with proper filtering
    std::vector<cv::Point> findBestDocument(const cv::Mat& binary, const cv::Mat& original) {
        std::vector<std::vector<cv::Point> > contours;
        std::vector<cv::Vec4i> hierarchy;

        cv::findContours(binary, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        std::vector<std::pair<double, std::vector<cv::Point> > > candidates;

        for (size_t i = 0; i < contours.size(); i++) {
            if (isValidDocument(contours[i], original.size())) {
                std::vector<cv::Point> approx;
                cv::approxPolyDP(contours[i], approx, config.epsilonFactor * cv::arcLength(contours[i], true), true);
                double area = cv::contourArea(approx);
                candidates.push_back(std::make_pair(area, approx));
            }
        }

        // Sort by area (largest first)
        for (size_t i = 0; i < candidates.size(); i++) {
            for (size_t j = i + 1; j < candidates.size(); j++) {
                if (candidates[i].first < candidates[j].first) {
                    std::swap(candidates[i], candidates[j]);
                }
            }
        }

        if (!candidates.empty()) {
            return candidates[0].second;
        }
        return std::vector<cv::Point>();
    }
};

// Document warper with good quality
class BalancedDocumentWarper {
public:
    static cv::Mat warpDocument(const cv::Mat& img, const std::vector<cv::Point>& points, bool enhance = true) {
        if (points.size() != 4) return cv::Mat();

        Config config;
        BalancedDocumentDetector detector(config);
        std::vector<cv::Point> ordered = detector.reorderPoints(points);

        double w1 = cv::norm(ordered[1] - ordered[0]);
        double w2 = cv::norm(ordered[3] - ordered[2]);
        double h1 = cv::norm(ordered[2] - ordered[0]);
        double h2 = cv::norm(ordered[3] - ordered[1]);

        int maxW = (w1 > w2) ? (int)w1 : (int)w2;
        int maxH = (h1 > h2) ? (int)h1 : (int)h2;

        if (maxW < 300) maxW = 300;
        if (maxH < 300) maxH = 300;

        cv::Point2f src[4] = { ordered[0], ordered[1], ordered[2], ordered[3] };
        cv::Point2f dst[4] = {
            cv::Point2f(0, 0), cv::Point2f((float)maxW, 0),
            cv::Point2f(0, (float)maxH), cv::Point2f((float)maxW, (float)maxH)
        };

        cv::Mat transform = cv::getPerspectiveTransform(src, dst);
        cv::Mat warped;
        cv::warpPerspective(img, warped, transform, cv::Size(maxW, maxH), cv::INTER_CUBIC);

        if (enhance) {
            return enhanceDocument(warped);
        }
        return warped;
    }

private:
    static cv::Mat enhanceDocument(const cv::Mat& img) {
        cv::Mat enhanced, gray;

        if (img.channels() == 3) {
            cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
        }
        else {
            gray = img.clone();
        }

        cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(3.0, cv::Size(8, 8));
        clahe->apply(gray, enhanced);
        cv::GaussianBlur(enhanced, enhanced, cv::Size(3, 3), 0.5);

        if (img.channels() == 3) {
            cv::cvtColor(enhanced, enhanced, cv::COLOR_GRAY2BGR);
        }
        return enhanced;
    }
};

// RESTORED: Proper quality assessment
QualityMetrics assessQuality(const cv::Mat& img) {
    QualityMetrics metrics;
    cv::Mat gray;

    if (img.channels() == 3) {
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    }
    else {
        gray = img.clone();
    }

    // Sharpness (Laplacian variance)
    cv::Mat laplacian;
    cv::Laplacian(gray, laplacian, CV_64F);
    cv::Scalar mean, stddev;
    cv::meanStdDev(laplacian, mean, stddev);
    metrics.sharpness = stddev[0] * stddev[0];

    // Brightness
    cv::meanStdDev(gray, mean, stddev);
    metrics.brightness = mean[0];

    // Contrast
    metrics.contrast = stddev[0];

    // Overall score calculation
    double sharpnessScore = (metrics.sharpness > 100.0) ? 100.0 : (metrics.sharpness / 100.0 * 100.0);
    double brightnessScore = 100.0 - (metrics.brightness > 128.0 ? (metrics.brightness - 128.0) : (128.0 - metrics.brightness)) / 128.0 * 100.0;
    double contrastScore = (metrics.contrast > 50.0) ? 100.0 : (metrics.contrast / 50.0 * 100.0);

    metrics.overallScore = (int)((sharpnessScore * 0.4 + brightnessScore * 0.3 + contrastScore * 0.3));
    metrics.isGoodQuality = metrics.overallScore >= 60; // Lowered threshold

    return metrics;
}

std::string getTimestamp() {
    std::time_t now = std::time(0);
    std::tm tm;
    localtime_s(&tm, &now);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");
    return oss.str();
}

long long getCurrentTimeMillis() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

// Enhanced UI with detection info
void drawUI(cv::Mat& img, const std::vector<cv::Point>& document,
    long long detectionStartTime, bool saved, int requiredSeconds,
    const QualityMetrics* quality, bool documentDetected = false, int fps = 0) {

    // Draw document outline
    if (document.size() == 4) {
        cv::polylines(img, document, true, cv::Scalar(0, 255, 0), 2);

        // Show document center
        cv::Point center(0, 0);
        for (size_t i = 0; i < document.size(); i++) {
            center.x += document[i].x;
            center.y += document[i].y;
        }
        center.x /= 4;
        center.y /= 4;
        cv::circle(img, center, 5, cv::Scalar(0, 255, 0), -1);
    }

    // Status with countdown
    std::string status;
    cv::Scalar statusColor;

    if (documentDetected && document.size() == 4) {
        if (saved) {
            status = "DOCUMENT SAVED!";
            statusColor = cv::Scalar(0, 255, 0);
        }
        else {
            long long elapsedTime = getCurrentTimeMillis() - detectionStartTime;
            int remainingSeconds = requiredSeconds - (int)(elapsedTime / 1000);
            if (remainingSeconds < 0) remainingSeconds = 0;

            std::ostringstream oss;
            oss << "Saving in " << remainingSeconds << " seconds...";
            status = oss.str();
            statusColor = cv::Scalar(0, 255, 255);
        }
    }
    else {
        status = "Searching for documents...";
        statusColor = cv::Scalar(0, 0, 255);
    }

    cv::putText(img, status, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.7, statusColor, 2);

    // Quality indicator
    if (quality != NULL && document.size() == 4) {
        std::ostringstream qualityOss;
        qualityOss << "Quality: " << quality->overallScore << "%";
        std::string qualityText = qualityOss.str();
        cv::Scalar qualityColor = quality->isGoodQuality ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255);
        cv::putText(img, qualityText, cv::Point(10, 60), cv::FONT_HERSHEY_SIMPLEX, 0.6, qualityColor, 2);
    }

    // FPS counter
    if (fps > 0) {
        std::ostringstream fpsOss;
        fpsOss << "FPS: " << fps;
        cv::putText(img, fpsOss.str(), cv::Point(10, img.rows - 50), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
    }

    // Controls
    cv::putText(img, "Press 'c' to capture manually, 'q' to quit",
        cv::Point(10, img.rows - 20), cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(255, 255, 255), 1);
}

int main() {
    Config config;

    std::cout << "=== BALANCED Document Scanner (Fast + Good Detection) ===" << std::endl;
    std::cout << "Resolution: " << config.frameWidth << "x" << config.frameHeight << std::endl;
    std::cout << "Min Area: " << config.minArea << " (adjusted for resolution)" << std::endl;
    std::cout << "Quality Threshold: " << config.qualityThreshold << "%" << std::endl;

    ensureDirectoryExists(config.saveFolder);

    SerialPort serial;
    bool serialOk = serial.init(config.comPort);
    std::cout << "Serial: " << (serialOk ? "? Connected" : "? Failed") << std::endl;

    // Camera setup with speed optimizations but stable settings
    cv::VideoCapture cap;

    cap.open(config.streamUrl, cv::CAP_FFMPEG);
    if (!cap.isOpened()) {
        std::cout << "IP camera failed, trying webcam..." << std::endl;
        cap.open(0, cv::CAP_DSHOW);
        if (!cap.isOpened()) {
            std::cerr << "No camera available!" << std::endl;
            return -1;
        }
    }

    // BALANCED camera settings
    cap.set(cv::CAP_PROP_FRAME_WIDTH, config.frameWidth);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, config.frameHeight);
    cap.set(cv::CAP_PROP_FPS, config.fps);
    cap.set(cv::CAP_PROP_BUFFERSIZE, config.bufferSize);

    std::cout << "?? Camera configured for balanced performance!" << std::endl;

    BalancedDocumentDetector detector(config);

    cv::Mat frame, processed, paperMask, combined;
    long long detectionStartTime = 0;
    bool documentDetected = false;
    bool saved = false;
    std::vector<cv::Point> lastBestDocument;
    int docCount = 0;

    // FPS calculation
    auto lastFpsTime = std::chrono::steady_clock::now();
    int fpsCounter = 0;
    int currentFps = 0;

    std::cout << "?? Starting balanced capture..." << std::endl;

    while (true) {
        cap >> frame;
        if (frame.empty()) continue;

        fpsCounter++;

        // Calculate FPS
        auto currentTime = std::chrono::steady_clock::now();
        auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastFpsTime);
        if (timeDiff.count() >= 1000) {
            currentFps = fpsCounter;
            fpsCounter = 0;
            lastFpsTime = currentTime;
        }

        cv::flip(frame, frame, 1);

        // FULL processing every frame for good detection
        processed = detector.balancedPreprocess(frame);

        // RESTORED: Color-based paper detection
        if (config.useColorDetection) {
            paperMask = detector.detectPaper(frame);
            cv::bitwise_and(processed, paperMask, combined);
        }
        else {
            combined = processed;
        }

        // Find document
        std::vector<cv::Point> document = detector.findBestDocument(combined, frame);

        QualityMetrics quality;
        bool hasGoodDocument = false;

        if (document.size() == 4) {
            cv::Mat warped = BalancedDocumentWarper::warpDocument(frame, document, false);
            if (!warped.empty()) {
                quality = assessQuality(warped);
                hasGoodDocument = quality.isGoodQuality;
            }
        }

        // Time-based detection logic
        bool currentFrameValid = !document.empty() && hasGoodDocument;

        if (currentFrameValid) {
            if (!documentDetected) {
                documentDetected = true;
                detectionStartTime = getCurrentTimeMillis();
                lastBestDocument = document;
                std::cout << "?? Document detected! Area: " << (int)cv::contourArea(document) << " Quality: " << quality.overallScore << "%" << std::endl;
                serial.send("DOC_DETECTED\n");
            }
            lastBestDocument = document;

        }
        else {
            if (documentDetected) {
                documentDetected = false;
                saved = false;
                std::cout << "? Detection lost!" << std::endl;
                serial.send("DOC_LOST\n");
            }
        }

        // Auto-save logic
        if (documentDetected && !saved) {
            long long elapsedTime = getCurrentTimeMillis() - detectionStartTime;
            if (elapsedTime >= (config.detectionTimeSeconds * 1000)) {
                cv::Mat warped = BalancedDocumentWarper::warpDocument(frame, lastBestDocument, config.autoEnhance);
                if (!warped.empty()) {
                    std::ostringstream filenameOss;
                    filenameOss << config.saveFolder << "doc_" << getTimestamp() << "_" << (++docCount) << ".jpg";
                    std::string filename = filenameOss.str();
                    cv::imwrite(filename, warped);

                    std::cout << "? Document saved: " << filename << std::endl;
                    std::cout << "?? Final quality: " << quality.overallScore << "%" << std::endl;

                    serial.send("DOC_SAVED\n");
                    saved = true;

                    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
                    documentDetected = false;
                    saved = false;
                }
            }
        }

        // Enhanced UI
        drawUI(frame, document, detectionStartTime, saved, config.detectionTimeSeconds,
            (document.size() == 4) ? &quality : NULL, documentDetected, currentFps);

        cv::imshow("BALANCED Document Scanner", frame);
        cv::imshow("Processing", combined); // Show processing result

        int key = cv::waitKey(1) & 0xFF;
        if (key == 'q' || key == 27) {
            break;
        }
        else if (key == 'c' && document.size() == 4) {
            cv::Mat warped = BalancedDocumentWarper::warpDocument(frame, document, config.autoEnhance);
            if (!warped.empty()) {
                std::ostringstream filenameOss;
                filenameOss << config.saveFolder << "manual_" << getTimestamp() << "_" << (++docCount) << ".jpg";
                std::string filename = filenameOss.str();
                cv::imwrite(filename, warped);
                std::cout << "?? Manual capture: " << filename << std::endl;
                serial.send("DOC_SAVED\n");
            }
        }
    }

    std::cout << "\n?? Shutting down..." << std::endl;
    serial.send("SCANNER_OFF\n");

    cap.release();
    cv::destroyAllWindows();

    std::cout << "?? Total documents: " << docCount << std::endl;
    return 0;
}
