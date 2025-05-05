INTRODUCTION :

In today’s fast-moving industrial and commercial world, effective warehouse management is essential for reducing operational costs, improving accuracy, and increasing productivity. Traditional inventory tracking systems often rely on manual labor, which can lead to errors, misplacements, and delays.
This project aims to address these issues by developing an Automated Inventory Verification and Tracking System using IoT technology. The system verifies incoming products against a cloud database, assigns them appropriate storage locations, and updates inventory status in real-time. Unverified products are rejected, ensuring only authorized goods are handled.
The integration of automation not only improves the speed and accuracy of warehouse operations but also reduces manual effort and increases efficiency. This solution has potential applications in logistics, retail, and manufacturing industries, where reliable inventory management is critical.

METHODOLOGY :

The system is designed to automate inventory verification and slot allocation in warehouses using IoT technology. At the core of the setup is the ESP32-WROOM-DA microcontroller, which handles all communication between the components and the cloud database.
The process begins when a product is placed on the conveyor. An infrared (IR) sensor detects its presence and activates the system from sleep mode. The product then reaches the RFID reader, which scans its unique ID tag. This ID is sent to a Firebase cloud database using Wi-Fi, where the product is verified.
If the product is found in the database, it is marked as valid, and a storage slot number is assigned. This slot number is displayed on an LCD screen for visual confirmation. If the product is not found, it is rejected, and no further action is taken.
As the verified product moves forward, a second IR sensor detects it near the end of the conveyor. This triggers a servo motor to open a small gate. Once the product passes through the gate, the servo closes it, guiding the item into its assigned storage slot.
The entire system is programmed using the Arduino IDE, and a custom Android app—developed using Kotlin and XML in Android Studio—is used to monitor product status in real time. This app connects to the same Firebase database and allows warehouse staff or vendors to track inventory from their smartphones.
By combining sensors, cloud communication, and automation, the system ensures accurate product verification, reduces manual errors, and improves overall warehouse efficiency.	

RESULTS  AND  DISCUSSIONS :

The proposed Automated Inventory Verification and Tracking System was successfully designed, implemented, and tested using the ESP32-WROOM-DA microcontroller. The system automates the process of product verification, slot allocation, and inventory tracking, aiming to improve the efficiency of warehouse operations.
3	Observed Output and System Functionality:
•	When a product is placed on the conveyor, the first IR sensor detects its presence and activates the system from sleep mode.
•	The RFID module reads the product’s ID and sends it to the Firebase cloud database through Wi-Fi.
•	If the product is authenticated, Firebase returns a slot number, which is displayed on the 16x2 LCD.
•	As the product continues, the second IR sensor detects it again and triggers the servo motor to open the gate.
•	The gate closes after the product passes, guiding it into its assigned slot for storage.
4	Mobile App Support:
A custom-built Android application, developed using Kotlin and XML in Android Studio, is connected to Firebase. It provides live updates about the product’s verification status, storage location, and availability, making the system user-friendly and accessible on smartphones.
5	Performance Highlights:
•	System Responsiveness: The ESP32 handled sensor inputs and cloud communication in real-time without noticeable delay.
•	Automation Accuracy: Valid products were identified and placed accurately into assigned slots. Invalid products were effectively filtered.
•	Power Efficiency: The sleep/wake feature helped minimize unnecessary power usage during idle periods.
•	Cloud Integration: Firebase ensured real-time data sync between the microcontroller and mobile application.
•	User Interaction: The LCD gave immediate feedback, and the mobile app extended the system’s accessibility and control.
6	 Challenges and Opportunities for Improvement:
•	The system currently supports one lane of product flow. Scalability to multiple conveyors could make it suitable for larger warehouses.
•	Adding more precise sensors could further enhance product detection and handling.
•	Incorporating a motorized conveyor belt would automate product movement more fully, rather than relying on manual placement.
The working model demonstrated consistent and reliable performance during testing, showing strong potential for real-world application in warehouse automation.


CONCLUSION :

The Automated Inventory Verification and Tracking System using the ESP32-WROOM-DA microcontroller successfully automates key warehouse tasks like product verification, slot allocation, and inventory tracking. By using RFID, Firebase, and a mobile app, the system reduces manual errors and makes warehouse operations more efficient.
The system worked well during testing, providing real-time updates on product status and accurately guiding items to their assigned storage slots. The mobile app allowed for easy tracking and management of inventory from anywhere.
While the system is effective for a single conveyor line, future improvements could include supporting multiple conveyors, adding more sensors for obstacle detection, and integrating a motorized conveyor for better speed control. These upgrades could make the system more adaptable for larger, more complex warehouse environments.
Overall, this project is a great starting point for warehouse automation and offers a scalable solution that could improve inventory management in real-world applications.




