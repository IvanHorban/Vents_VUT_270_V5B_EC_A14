#include "blauberg_s14_controller.h"
#include "esphome/core/log.h"

namespace esphome {
    namespace blauberg_s14 {
        static const char *TAG = "BlaubergS14.controller";

        void BlaubergS14Controller::setCurrentSpeed(int currentSpeed) {
            switch (currentSpeed) {
                case 0:
                    this->currentSpeed = BlaubergS14Controller::S14_SPEED_0;
                break;
                
                case 1:
                    this->currentSpeed = BlaubergS14Controller::S14_SPEED_1;
                break;
                
                case 2:
                    this->currentSpeed = BlaubergS14Controller::S14_SPEED_2;
                break;
                
                case 3:
                    this->currentSpeed = BlaubergS14Controller::S14_SPEED_3;
                break;

                default:
                    this->currentSpeed = BlaubergS14Controller::S14_SPEED_0;
            }

            ESP_LOGD(TAG, "BlaubergS14Controller::setCurrentSpeed %d", this->currentSpeed);
        }

        void BlaubergS14Controller::setCurrentDamper(bool currentDamper) {
            this->currentDamper = currentDamper ? BlaubergS14Controller::S14_DAMPER_ON : BlaubergS14Controller::S14_DAMPER_OFF;

            if (this->sensor_damper_ != nullptr) {
                this->sensor_damper_->publish_state(this->currentDamper == BlaubergS14Controller::S14_DAMPER_ON);
            }

            ESP_LOGD(TAG, "BlaubergS14Controller::setCurrentDamper %d", (int) currentDamper);
        }

        void BlaubergS14Controller::setResetFilter(bool resetFilter) {
            this->currentResetFilter = resetFilter 
                ? BlaubergS14Controller::S14_RESET_FILTER_ON 
                : BlaubergS14Controller::S14_RESET_FILTER_OFF
            ;

            ESP_LOGD(TAG, "BlaubergS14Controller::setResetFilter %d", (int) resetFilter);
        }

        void BlaubergS14Controller::setup() {
            if (this->sensor_damper_ != nullptr) {
                this->sensor_damper_->publish_state(this->currentDamper == BlaubergS14Controller::S14_DAMPER_ON);
            }

            if (this->sensor_isDefrosting_ != nullptr) {
                this->sensor_isDefrosting_->publish_state(this->isDefrosting);
            }

            if (this->sensor_filterReplacementRequired_ != nullptr) {
                this->sensor_filterReplacementRequired_->publish_state(this->filterReplacementRequired);
            }

            if (this->sensor_response_ != nullptr) {
                this->sensor_response_->publish_state(std::to_string(BlaubergS14Controller::S14_RESPONSE_OK));
            }
        }

        void BlaubergS14Controller::loop() {
            uint32_t now = millis();

            if (
                this->isDefrosting
                && now - this->defrostingFromMillis > DEFROSTING_TIME
            ) {
                this->isDefrosting = false;

                if (this->sensor_isDefrosting_ != nullptr) {
                    this->sensor_isDefrosting_->publish_state(this->isDefrosting);
                }
            }

            if (this->terminated) {
                return;
            }

            if (
                this->lastResponseReceived
                && now - this->lastResponseReceivedAt >= 90
            ) {
                this->lastResponseReceived = false;

                write(this->currentSpeed | this->currentDamper | this->currentResetFilter);
            }

            int response = BlaubergS14Controller::S14_RESPONSE_OK;

            while (available()) {
                this->lastResponseReceived = true;
                this->lastResponseReceivedAt = millis();

                response = read();

                if (this->currentResetFilter) {
                    this->setResetFilter(false);
                    this->filterReplacementRequired = false;

                    if (this->sensor_filterReplacementRequired_ != nullptr) {
                        this->sensor_filterReplacementRequired_->publish_state(this->filterReplacementRequired);
                    }
                }

                if (this->lastResponse != response) {
                    this->lastResponse = response;

                    if (this->sensor_response_ != nullptr) {
                        this->sensor_response_->publish_state(std::to_string(this->lastResponse));
                    }

                    switch (this->lastResponse) {
                        case BlaubergS14Controller::S14_RESPONSE_OK:
                            break;
                        case BlaubergS14Controller::S14_RESPONSE_DEFROSTING_REQUIRED:
                            this->defrostingFromMillis = millis();
                            this->isDefrosting = true;

                            if (this->sensor_isDefrosting_ != nullptr) {
                                this->sensor_isDefrosting_->publish_state(this->isDefrosting);
                            }

                        break;

                        case BlaubergS14Controller::S14_RESPONSE_FILTER_REPLACEMENT_REQUIRED:
                            this->filterReplacementRequired = true;

                            if (this->sensor_filterReplacementRequired_ != nullptr) {
                                this->sensor_filterReplacementRequired_->publish_state(this->filterReplacementRequired);
                            }

                        break;
                    }
                }
            }
        }

    }  // namespace blauberg_s14
}  // namespace esphome